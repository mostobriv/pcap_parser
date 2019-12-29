#include "DatabaseWriter.h"

#include <set>
#include <tuple>
#include <string>
#include <cstring>

using namespace std::literals::chrono_literals;


const DatabaseWriter::ConnectionData DatabaseWriter::DefaultConnData =
    {"localhost", 5432, "pcap", "pcap_312b4a6b229587d831dd4a05fc83d4f7"};

logger::Logger DatabaseWriter::logger ("Database", logger::Level::LVL_DEBUG);


DatabaseWriter::DatabaseWriter( ThreadQueue<StreamData>& queue
                              , const ConnectionData& cdata
                              )
    : m_conn  (cdata.to_pq_string())
    , m_queue (queue)
{
    // Do auto migrations

    pqxx::work w (m_conn);
    pqxx::result res;

    // Create tables
    auto create_command =
        "CREATE TABLE IF NOT EXISTS streams "
        "( id SERIAL PRIMARY KEY"
        ", data BYTEA"
        ", replies INTEGER ARRAY"
        ");";
    logger.debug() << create_command;
    w.exec(create_command);

    // Check table columns
    res = w.exec(
        " SELECT column_name, data_type"
        " FROM information_schema.columns"
        " WHERE table_name = 'streams';"
    );

    std::set< std::tuple< std::string, std::string > > expected_cols =
        { {"id",       "integer"}
        , {"data",     "bytea"}
        , {"replies", "ARRAY"}
        };

    for (const auto& row : res) {
        const auto& name = row[0].as<std::string>();
        const auto& type = row[1].as<std::string>();

        auto it = expected_cols.find({name, type});
        if (it == expected_cols.end()) {
            logger.warning() << "Column `" << name << ":" << type
                << "` exists in database, but is not specified in migration policy";
        }
        expected_cols.erase(it);
    }

    // create missing columns
    for (const auto& [name, type] : expected_cols) {
        auto alter_command =
            " ALTER TABLE streams"
            " ADD COLUMN "
            + name + " " + type +
            ";";
        logger.debug() << alter_command;
        w.exec(alter_command);
    }

    w.commit();
}


bool DatabaseWriter::should_stop() const
{
    return m_should_stop;
}
DatabaseWriter& DatabaseWriter::set_should_stop(bool arg)
{
    m_should_stop = arg;
    return *this;
}


void DatabaseWriter::write()
{
    std::lock_guard _lock (m_mutex);
    logger.debug() << "wait for first stream";

    while (true) {
        auto mb_stream = m_queue.pop(1s);

        if (not mb_stream.has_value() and m_should_stop) {
            logger.info() << "shutting down writer";
            break;

        } else if (mb_stream.has_value()) {
            logger.debug() << "got stream";
            this->write_one(*mb_stream);
            logger.debug() << "wait for next stream";
        }
    }
}


void DatabaseWriter::write_one(const StreamData& stream)
{
    if (stream.data().empty()) {
        return;
    }
    // first we concatenate all streams and collect reply offsets into a
    // prepared string

    std::vector<unsigned char> concat;
    std::string switches;

    size_t size = 0;
    for (const auto& reply : stream.data()) {
        size += reply.size();
    }
    concat.resize(size);
    switches.reserve(size * 4 + 2); // 3 characters for a digit + a comma + braces

    int current_offset = 0;
    auto* concat_last = concat.data();
    switches.push_back('{');

    for (const auto& reply : stream.data()) {
        std::memcpy( concat_last, reply.data(), reply.size() );
        concat_last += reply.size();

        current_offset += reply.size();
        switches.append( std::to_string(current_offset) + "," );
    }
    switches.back() = '}';

    // create a binary string from concat and write that to db

    auto bin_str = pqxx::binarystring(concat.data(), concat.size());
    pqxx::work w (m_conn);
    auto query_text = "INSERT INTO streams (data, replies) VALUES ($1, $2)";
    logger.debug() << query_text;
    w.parameterized(query_text)(bin_str)(switches).exec();

    w.commit();
}
