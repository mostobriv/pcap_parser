#pragma once

#include "termcolor.hpp"

#include <stdexcept>
#include <iostream>
#include <fstream>

#include <fmt/format.h>

#include <boost/stacktrace.hpp>
#include <boost/exception/all.hpp>


enum LoggingLevel {LVL_DEBUG=0, LVL_INFO, LVL_WARNING, LVL_ERROR};

using traced = boost::error_info<struct tag_stacktrace, boost::stacktrace::stacktrace>;


class Logger {
    private:
        LoggingLevel log_level;
        std::string logger_name;
        std::ofstream file_stream;


    public:
        Logger(const char* const name="huypizda", LoggingLevel const level=LVL_DEBUG);
        ~Logger();

        Logger(const Logger&) = delete;
        void operator=(const Logger&) = delete;


        void debug(std::string msg);
        void info(std::string msg);
        void warning(std::string msg);
        void error(std::string msg);
        void exception(const std::exception&);
};