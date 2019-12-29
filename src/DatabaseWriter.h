#pragma once

// Description: acceps parsed streams from a queue and writes them to database
// Thread-safety: this class can be shared between threads, but only one should
// call the `write` function. The function locks, so if you do, it would still
// do nothing.
// Calling `write` will write forever. To terminate, wait for the associated
// queue to become empty, wait for the writer to become waiting, and terminate
// the writer thread.


#include <mutex>
#include <pqxx/pqxx>
#include "ThreadQueue.hpp"
#include "StreamData.h"
#include "logger.h"


class DatabaseWriter
{
    public:
        struct ConnectionData
        {
            std::string host;
            int         port;
            std::string user;
            std::string password;

            std::string to_pq_string() const
            {
                return  "host=" + host
                     + " port=" + std::to_string(port)
                     + " user=" + user
                     + " password=" + password;
            }
        };
        static const ConnectionData DefaultConnData;

        static logger::Logger logger;

    private:
        mutable std::mutex       m_mutex;
        pqxx::connection         m_conn;
        ThreadQueue<StreamData>& m_queue;
        bool                     m_is_waiting;

        void write_one(const StreamData&);

    public:
        DatabaseWriter( ThreadQueue<StreamData>&
                      , const ConnectionData& cdata = DefaultConnData
                      );
        DatabaseWriter() = delete;
        DatabaseWriter(const DatabaseWriter&) = delete;
        DatabaseWriter(DatabaseWriter&&) = delete;

        // shows that the thread can be terminated now
        bool is_waiting() const;

        void write();
};
