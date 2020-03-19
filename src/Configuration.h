#pragma once

#include <filesystem>
#include <string>
#include <optional>
#include <vector>

#include "logger.h"
#include "DatabaseWriter.h"


struct Configuration
{
    private:
        static logger::Logger<logger::Level::LVL_DEBUG> logger;
        // remember to update this when updating ConnectionData
        DatabaseWriter::ConnectionData m_incomplete_db_creds;
        bool m_has_db_host = false;
        bool m_has_db_port = false;
        bool m_has_db_user = false;
        bool m_has_db_pwd = false;

    public:
        bool                                 should_exit;
        std::string                          program_name;
        std::vector<std::filesystem::path>   dirs;
        std::vector<std::string>             files;
        std::optional<DatabaseWriter::ConnectionData> db_creds;

        Configuration(
              int argc, const char** argv
            , const std::filesystem::path& default_config_file = "config.yaml"
            );
};
