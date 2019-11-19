#include "logger.h"

#include <ctime>


#define CHECK_LOGLEVEL(dlvl) if (dlvl < log_level) { return; }


template <class E>
void throw_with_trace(const E& e) {
    throw boost::enable_error_info(e)
        << traced(boost::stacktrace::stacktrace());
}


Logger::Logger(const char* const name, LoggingLevel level) :
                    log_level(level),
                    logger_name(name)
{
    file_stream.open(fmt::format("logs/{}.log", name), 
                    std::ios::out | std::ios::app);
}


Logger::~Logger() {
    if (file_stream) {
        file_stream.close();
    }
}


void Logger::debug(std::string msg) { 
    CHECK_LOGLEVEL(LVL_DEBUG);
    std::time_t t = std::time(nullptr);
    char mbstr[128];

    std::strftime(mbstr, sizeof(mbstr), "%F %T", std::localtime(&t));
    

    file_stream << fmt::format("{} [DEBUG] [{}] {}", mbstr, logger_name, msg) << std::endl;
    std::cout << mbstr << " [" << termcolor::green << "DEBUG" << termcolor::reset << "] [" << logger_name << "] " << msg << std::endl;
}


void Logger::info(std::string msg) { 
    CHECK_LOGLEVEL(LVL_INFO);
    std::time_t t = std::time(nullptr);
    char mbstr[128];

    std::strftime(mbstr, sizeof(mbstr), "%F %T", std::localtime(&t));


    file_stream << fmt::format("{} [INFO] [{}] {}", mbstr, logger_name, msg) << std::endl;
    std::cout << mbstr << " [" << termcolor::yellow << "INFO" << termcolor::reset << "] [" << logger_name << "] " << msg << std::endl;
}


void Logger::warning(std::string msg) {
    CHECK_LOGLEVEL(LVL_WARNING);
    std::time_t t = std::time(nullptr);
    char mbstr[128];

    std::strftime(mbstr, sizeof(mbstr), "%F %T", std::localtime(&t));


    file_stream << fmt::format("{} [WARNING] [{}] {}", mbstr, logger_name, msg) << std::endl;
    std::cout << mbstr << " [" << termcolor::blue << "WARNING" << termcolor::reset << "] [" << logger_name << "] " << msg << std::endl;
}


void Logger::error(std::string msg) {
    CHECK_LOGLEVEL(LVL_ERROR);
    std::time_t t = std::time(nullptr);
    char mbstr[128];

    std::strftime(mbstr, sizeof(mbstr), "%F %T", std::localtime(&t));


    file_stream << fmt::format("{} [ERROR] [{}] {}", mbstr, logger_name, msg) << std::endl;
    std::cout << mbstr << " [" << termcolor::red << "ERROR" << termcolor::reset << "] [" << logger_name << "] " << msg << std::endl;
}


void Logger::exception(const std::exception& e) {
    const boost::stacktrace::stacktrace* st = boost::get_error_info<traced>(e);
    if (st) {
        std::cout << termcolor::red << *st << termcolor::reset << std::endl;
    }
}