#include "DatabaseWriter.h"

#include <set>
#include <tuple>
#include <string>


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


bool DatabaseWriter::is_waiting() const
{
    return false;
}


void DatabaseWriter::write()
{
    std::lock_guard _lock (m_mutex);
}


void DatabaseWriter::write_one(const ConnectionData&)
{
    std::lock_guard _lock (m_mutex);
}
