#include "writer.h"


const DatabaseWriter::ConnectionData DatabaseWriter::DefaultConnData =
    {"localhost", 5432, "pcap", "pcap_312b4a6b229587d831dd4a05fc83d4f7"};


DatabaseWriter::DatabaseWriter(const ConnectionData& cdata)
    : m_conn(cdata.to_pq_string())
{
}


bool DatabaseWriter::is_waiting() const
{
    return false;
}


void DatabaseWriter::write()
{
    std::lock_guard _lock (m_mutex);
}


void DatabaseWriter::write_one()
{
    std::lock_guard _lock (m_mutex);
}
