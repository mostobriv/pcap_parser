#pragma once

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <mutex>

#include <boost/stacktrace.hpp>
#include <boost/exception/all.hpp>

#include "termcolor.hpp"


namespace logger
{

enum Level {LVL_DEBUG=0, LVL_INFO, LVL_WARNING, LVL_ERROR, LVL_SILENT};

class __BaseLoggerLock
{
    // Description: used for logger-global mutex
    protected:
        static std::mutex logger_mutex;
};

template <Level loglevel>
class Logger : __BaseLoggerLock
{
    // stream that automatically inserts newline when writing is finished
    struct logstream
    {
        bool alive;
        Logger& logger;
        inline logstream(Logger& a_logger)
            : alive(true)
            , logger(a_logger)
            {}
        inline logstream(logstream&& other)
            : alive(other.alive)
            , logger(other.logger)
            {other.alive = false;}
        logstream(const logstream&) = delete;
        inline ~logstream() {if (alive) logger.endl();}
    };
    template <typename T> friend inline logstream operator<< (logstream&& stream, const T& x)
    {
        if (not stream.alive)  return std::move(stream);
        stream.logger.append_separated(x);
        return std::move(stream);
    }

    private:
        std::string   m_name;
        std::ofstream m_file_stream;

        // generic printing, combined by public logging functions
        template <Level lvl> void header();
        template <Level lvl> logstream log();
        inline void endl();
        template <typename T, Level lvl> logstream log(const T&);
        template <typename T> void append(const T&);
        template <typename T> void append_separated(const T&);

    public:
        Logger(const std::string& name);

        Logger() = delete;
        Logger(const Logger&) = delete;
        Logger(Logger&&) = delete;
        void operator=(const Logger&) = delete;
        void operator=(Logger&&) = delete;


        // Main logging functions. Can be used to log one thing or to start
        // logging stream. Stream has newline inserted automatically at the end
        inline logstream debug()   {return log<LVL_DEBUG>();}
        inline logstream info()    {return log<LVL_INFO>();}
        inline logstream warning() {return log<LVL_WARNING>();}
        inline logstream error()   {return log<LVL_ERROR>();}
        template <typename T> logstream debug(const T& x)
            {return log(LVL_DEBUG, x);}
        template <typename T> logstream info(const T& x)
            {return log(LVL_INFO, x);}
        template <typename T> logstream warning(const T& x)
            {return log(LVL_WARNING, x);}
        template <typename T> logstream error(const T& x)
            {return log(LVL_ERROR, x);}

        // Specifically for exception. Prints
        void exception(const std::exception&);
};

// Global logger
extern Logger<LVL_DEBUG> logger;


template <Level loglevel>
template <Level cur_level>
inline typename Logger<loglevel>::logstream Logger<loglevel>::log()
{
    std::lock_guard _lock (logger_mutex);

    header<cur_level>();
    logstream stream (*this);
    if (cur_level < loglevel) {
        stream.alive = false;
    }
    return stream;
}

template <Level loglevel>
template <typename T, Level cur_level>
inline typename Logger<loglevel>::logstream Logger<loglevel>::log(const T& x)
{
    auto&& stream = log(cur_level);
    if (cur_level >= loglevel) {
        append_separated(x);
    }
    return std::move(stream);
}

template <Level loglevel>
inline void Logger<loglevel>::endl()
{
    std::cerr << std::endl;
    if (m_file_stream.is_open()) {
        m_file_stream << std::endl;
    }
}

template <Level loglevel>
template <typename T>
inline void Logger<loglevel>::append(const T& x)
{
    std::cerr << x;
    if (m_file_stream.is_open()) {
        m_file_stream << x;
    }
}
template <Level loglevel>
template <typename T>
inline void Logger<loglevel>::append_separated(const T& x)
{
    std::cerr << ' ' << x;
    if (m_file_stream.is_open()) {
        m_file_stream << ' ' << x;
    }
}


template <Level loglevel>
Logger<loglevel>::Logger(const std::string& name)
    : m_name (name)
{
    m_file_stream.open("logs/" + name + ".log"
                      ,std::ios::out | std::ios::app);
}


template <Level loglevel>
template <Level cur_level>
void Logger<loglevel>::header()
{
    if (cur_level < loglevel)  return;
    // print current time
    auto time = std::chrono::system_clock::now();
    auto time_c = std::chrono::system_clock::to_time_t(time);
    append(std::put_time(std::localtime(&time_c), "%F %T"));

    // print colored level info
    append(" [");
    switch (cur_level) {
        case LVL_DEBUG:
            append(termcolor::green);
            append(" DEBUG ");
            break;
        case LVL_INFO:
            append(termcolor::cyan);
            append(" INFO  ");
            break;
        case LVL_WARNING:
            append(termcolor::yellow);
            append("WARNING");
            break;
        case LVL_ERROR:
            append(termcolor::red);
            append(" ERROR ");
            break;
        default:
            break;
    }
    append(termcolor::reset);
    append("] [");

    append(m_name);
    append("]");
}


template <Level loglevel>
void Logger<loglevel>::exception(const std::exception& e)
{
    const boost::stacktrace::stacktrace* st =
        boost::get_error_info<
            boost::error_info<struct tag_stacktrace, boost::stacktrace::stacktrace>
        >(e);
    (st ? error() << *st
        : error() << "Exception:"
        ) << e.what();
}

}; // namespace logger
