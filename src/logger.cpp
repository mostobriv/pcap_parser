#include "logger.h"

#include <stdexcept>
#include <chrono>

#include <fmt/format.h>

#include <boost/stacktrace.hpp>
#include <boost/exception/all.hpp>

#include "termcolor.hpp"


namespace logger
{

using traced = boost::error_info<struct tag_stacktrace, boost::stacktrace::stacktrace>;


template <class E>
void throw_with_trace(const E& e) 
{
    throw boost::enable_error_info(e)
        << traced(boost::stacktrace::stacktrace());
}


Logger::Logger(const std::string& name, Level level) :
                    m_level(level),
                    m_name(name)
{
    m_file_stream.open(fmt::format("logs/{}.log", name)
                      ,std::ios::out | std::ios::app);
}


void Logger::header(Level lvl)
{
    if (lvl < m_level)  return;
    // print current time
    auto time = std::chrono::system_clock::now();
    auto time_c = std::chrono::system_clock::to_time_t(time);
    append(std::put_time(std::localtime(&time_c), "%F %T"));

    // print colored level info
    append(" [");
    switch (lvl) {
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
    append("] ");
}


void Logger::exception(const std::exception& e)
{
    const boost::stacktrace::stacktrace* st = boost::get_error_info<traced>(e);
    if (st) {
        std::cout << termcolor::red << *st << termcolor::reset << std::endl;
    }
}

} // namespace logger
