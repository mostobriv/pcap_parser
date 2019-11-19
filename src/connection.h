#pragma once

#include <memory>
#include <mutex>
#include <libpq-fe.h>


class PGConnection {
    public:
        PGConnection();
        std::shared_ptr<PGconn> connection() const;

    private:
        void establish_connection();

        std::string m_dbhost = "database";
        int         m_dbport = 5432;
        std::string m_dbname = "test";
        std::string m_dbuser = "test";
        std::string m_dbpass = "test";

        std::shared_ptr<PGconn>  m_connection;

};