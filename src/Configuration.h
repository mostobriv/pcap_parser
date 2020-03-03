#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>


struct Configuration
{
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
