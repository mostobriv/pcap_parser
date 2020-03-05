#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>

#include "logger.h"


struct Configuration
{
    private:
        static logger::Logger<logger::Level::LVL_DEBUG> logger;

    public:
        bool                                 should_exit;
        std::string                          program_name;
        std::vector<boost::filesystem::path> dirs;
        std::vector<std::string>             files;

        Configuration(
              int argc, const char** argv
            , const std::filesystem::path& default_config_file = "config.yaml"
            );
};
