#include "Configuration.h"

#include <iostream>
#include <string_view>
#include <optional>
#include <string_view>
#include <tuple>


logger::Logger<logger::Level::LVL_DEBUG> Configuration::logger ("Conf");


bool has_prefix(const std::string& prefix, const std::string& str)
{
    if (str.size() < prefix.size())  return false;
    auto begins = std::string_view(str).substr(0, prefix.size());
    return begins == prefix;
}


// Command line option which has a string value
struct ValueOption
{
    std::string long_form;
    std::string short_form;

    bool matches(const std::string& str)
    {
        return has_prefix(short_form, str) or has_prefix(long_form, str);
    }

    auto get(int argc, const char** argv)
        -> std::tuple<int, const char**, bool, std::optional<std::string>>
           /*         argc   argv'     matched       match val          */
    {
        std::string str = *argv;
        if (str == short_form or str == long_form) {
            return get_next(argc, argv);
        } else if (has_prefix(short_form, str)) {
            return { argc, argv, true, str.substr(short_form.size()) };
        } else if (has_prefix(long_form + "=", str)) {
            return { argc, argv, true, str.substr(long_form.size() + 1) };
        } else {
            return {argc, argv, false, {}};
        }
    }

    private:
        auto get_next(int argc, const char** argv)
            -> std::tuple<int, const char**, bool, std::optional<std::string>>
        {
            argc -= 1;
            argv += 1;
            if (argc == 0) {
                return {argc, argv, true, {}};
            } else {
                return {argc, argv, true, {*argv}};
            }
        }
};


bool is_help(const char* str)
{
    std::string_view s (str);

    return s == "-h" or s == "--help";
}
void print_help(const std::string& name)
{
    std::cout
        << "Usage: " << name << " [--dir=DIR...] PCAP..."
        << "\nPahan pcap loader"
        << "\n"
        << "\n    DIR - directory to watch for new pcaps"
        << "\n"
        << "\n    PCAP - pcap-file to add immediately"
        << std::endl;
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
    ValueOption dir_opt = {"--dir", "-d"};
    argc -= 1; //
    argv += 1; // skips argv[0]

    while (argc > 0) {
        auto cur_argc = argc;
        auto cur_argv = argv;
        argc -= 1;
        argv += 1;

        logger.debug() << cur_argc << "options left";
        logger.debug() << "Parsing option" << *cur_argv;


        if (is_help(*cur_argv)) {
            should_exit = true;
            print_help(program_name);

            return;
        }

        if (is_version(*cur_argv)) {
            should_exit = true;
            print_version(program_name);

            return;
        }

        logger.debug() << "Trying dir option";
        auto [argc_next, argv_next, matched, val] = dir_opt.get(cur_argc, cur_argv);
        argc = argc_next - 1;
        argv = argv_next + 1;
        if (matched) {
            if (val.has_value()) {
                logger.debug() << "Add directory" << *val;
                dirs.push_back(*val);
            } else {
                std::cout
                    << "Incorrect usage of options. Type --help for more info."
                    << std::endl;
                should_exit = true;
                return;
            }

            continue;
        }

        logger.debug() << "Didn't match any named option";

        // All positional options are files
        files.push_back(*cur_argv);
        logger.debug() << "Add file" << *cur_argv;
    }
}
