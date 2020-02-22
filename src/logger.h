#pragma once

#include <iostream>
#include <fstream>


namespace logger
{

enum Level {LVL_DEBUG=0, LVL_INFO, LVL_WARNING, LVL_ERROR, LVL_SILENT};

class Logger
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
    template<typename T> friend logstream operator<< (logstream&&, const T&);

    private:
        Level         m_level;
        std::string   m_name;
        std::ofstream m_file_stream;

        // generic printing, combined by public logging functions
        void header(Level);
        inline logstream log(Level);
        inline void endl();
        template<typename T> logstream log(Level, const T&);
        template<typename T> void append(const T&);
        template<typename T> void append_separated(const T&);

    public:
        Logger(const std::string& name, const Level=LVL_INFO);

        Logger() = delete;
        Logger(const Logger&) = delete;
        Logger(Logger&&) = delete;
        void operator=(const Logger&) = delete;
        void operator=(Logger&&) = delete;

        inline void set_log_level(Level lvl) {m_level = lvl;}


        // Main logging functions. Can be used to log one thing or to start
        // logging stream. Stream has newline inserted automatically at the end
        inline logstream debug()   {return log(LVL_DEBUG);}
        inline logstream info()    {return log(LVL_INFO);}
        inline logstream warning() {return log(LVL_WARNING);}
        inline logstream error()   {return log(LVL_ERROR);}
        template<typename T> logstream debug(const T& x)
            {return log(LVL_DEBUG, x);}
        template<typename T> logstream info(const T& x)
            {return log(LVL_INFO, x);}
        template<typename T> logstream warning(const T& x)
            {return log(LVL_WARNING, x);}
        template<typename T> logstream error(const T& x)
            {return log(LVL_ERROR, x);}

        // Specifically for exception. Prints
        void exception(const std::exception&);
};

// Global logger
extern Logger logger;


template<typename T>
inline Logger::logstream operator<<(Logger::logstream&& stream, const T& x)
{
    if (not stream.alive)  return std::move(stream);
    stream.logger.append_separated(x);
    return std::move(stream);
}


inline Logger::logstream Logger::log(Level lvl)
{
    header(lvl);
    logstream stream (*this);
    if (lvl < m_level) {
        stream.alive = false;
    }
    return std::move(stream);
}

template<typename T>
inline Logger::logstream Logger::log(Level lvl, const T& x)
{
    auto&& stream = log(lvl);
    if (lvl >= m_level) {
        append_separated(x);
    }
    return std::move(stream);
}

inline void Logger::endl()
{
    std::cerr << std::endl;
    if (m_file_stream.is_open()) {
        m_file_stream << std::endl;
    }
}

template<typename T>
inline void Logger::append(const T& x)
{
    std::cerr << x;
    if (m_file_stream.is_open()) {
        m_file_stream << x;
    }
}
template<typename T>
inline void Logger::append_separated(const T& x)
{
    std::cerr << ' ' << x;
    if (m_file_stream.is_open()) {
        m_file_stream << ' ' << x;
    }
}

}; // namespace logger
