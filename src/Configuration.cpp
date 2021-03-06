#include "Configuration.h"

#include <iostream>
#include <string_view>
#include <unordered_map>
#include <tuple>
#include "yaml-cpp/yaml.h"


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

    bool matches(const std::string& str) const
    {
        return has_prefix(short_form, str) or has_prefix(long_form, str);
    }

    auto get(int argc, const char** argv) const
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

    bool operator== (const ValueOption& rhs) const
    {
        return this->long_form == rhs.long_form;
    }

    private:
        auto get_next(int argc, const char** argv) const
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

namespace std
{
    template<> struct hash<ValueOption>
    {
        std::size_t operator() (const ValueOption& v) const noexcept
        {
            return std::hash<std::string>()(v.long_form);
        }
    };
}


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
    , db_creds     ({})
{
    // create named options

    ValueOption dir_opt = {"--dir", "-d"};
    auto on_dir_opt = [&](const std::string& val) {
        logger.debug() << "Add directory" << val;
        dirs.push_back(val);
    };

    ValueOption db_host_opt {"--db-host", "--db-host="};
    auto on_db_host = [&](const std::string& val) {
        logger.debug() << "Set db host to" << val;
        m_incomplete_db_creds.host = val;
        m_has_db_host = true;
    };
    ValueOption db_port_opt {"--db-port", "--db-port="};
    auto on_db_port = [&](const std::string& val) {
        logger.debug() << "Set db port to" << val;
        m_incomplete_db_creds.port = std::stoi(val);
        m_has_db_port = true;
    };
    ValueOption db_user_opt {"--db-user", "--db-user="};
    auto on_db_user = [&](const std::string& val) {
        logger.debug() << "Set db user to" << val;
        m_incomplete_db_creds.user = val;
        m_has_db_user = true;
    };
    ValueOption db_pwd_opt {"--db-pwd", "--db-pwd="};
    auto on_db_pwd = [&](const std::string& val) {
        logger.debug() << "Set db pwd to" << val;
        m_incomplete_db_creds.password = val;
        m_has_db_pwd = true;
    };

    const std::unordered_map
        <ValueOption, std::function<void(const std::string&)>> options =
        { {dir_opt, on_dir_opt}
        , {db_host_opt, on_db_host}
        , {db_port_opt, on_db_port}
        , {db_user_opt, on_db_user}
        , {db_pwd_opt, on_db_pwd}
        };

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

        bool value_option_matched = false;
        for (const auto& [opt, cb] : options) {
            auto [argc_next, argv_next, matched, val] =
                opt.get(cur_argc, cur_argv);
            argc = argc_next - 1;
            argv = argv_next + 1;

            if (matched) {
                if (val.has_value()) {
                    cb(*val);
                    value_option_matched = true;
                    break;
                } else {
                    std::cout
                        << "Incorrect usage of options."
                        << " Type --help for more info."
                        << std::endl;
                    should_exit = true;
                }
            }
            if (should_exit)  return;
        }

        if (not value_option_matched) {
            logger.debug() << "Didn't match any named option";

            // All positional options are files
            files.push_back(*cur_argv);
            logger.debug() << "Add file" << *cur_argv;
        }
    }

    // finalize connection data
    if (m_has_db_host and m_has_db_port and m_has_db_user and m_has_db_pwd) {
        logger.debug() << "Setting full connection data";
        db_creds = m_incomplete_db_creds;
    }

    if (std::filesystem::exists(default_config_file)) {
        auto f = YAML::LoadFile(default_config_file);
    }
}
