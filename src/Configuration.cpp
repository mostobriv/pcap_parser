#include "Configuration.h"

#include <iostream>
#include <string_view>


bool is_help(const char* str)
{
    std::string_view s (str);

    return s == "-h" or s == "--help";
}
void print_help(const std::string& name)
{
    std::cout << "help, i'm trapped in a help machine!" << std::endl;
}

bool is_version(const char* str)
{
    std::string_view s (str);

    return s == "-v" or s == "--version";
}
void print_version(const std::string& name)
{
    std::cout << name << ", version 0.0.0" << std::endl;
}


Configuration::Configuration(
      int argc, const char** argv
    , const std::filesystem::path& default_config_file
    )
    : should_exit  (false)
    , program_name (argv[0])
{
    int count = 1; // skip argv[0]

    while (count < argc) {
        count += 1;
        argv += 1;
        auto* str = *argv;

        if (is_help(str)) {
            should_exit = true;
            print_help(program_name);
            return;
        }

        if (is_version(str)) {
            should_exit = true;
            print_version(program_name);
            return;
        }
    }
}
