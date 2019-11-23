#pragma once

#include <iostream>
#include <fstream>


namespace logger
{

enum Level {LVL_DEBUG=0, LVL_INFO, LVL_WARNING, LVL_ERROR, LVL_SILENT};

class Logger
{
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
    friend class logstream;
    template<typename T> friend logstream operator<< (logstream&&, const T&);

    private:
        Level  m_level;
        std::string   m_name;
        std::ofstream m_file_stream;

        void header(Level);
        inline logstream log(Level);
        inline void endl();
        template<typename T> logstream log(Level, const T&);
        template<typename T> void append(const T&);

    public:
        Logger(const std::string& name, const Level=LVL_INFO);

        Logger() = delete;
        Logger(const Logger&) = delete;
        Logger(Logger&&) = delete;
        void operator=(const Logger&) = delete;
        void operator=(Logger&&) = delete;

        inline void set_log_level(Level lvl) {m_level = lvl;}


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
        void exception(const std::exception&);
};

//Logger logger; // global logger


template<typename T>
Logger::logstream operator<<(Logger::logstream&& stream, const T& x)
{
    if (not stream.alive)  return std::move(stream);
    stream.logger.append(x);
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
Logger::logstream Logger::log(Level lvl, const T& x)
{
    auto&& stream = log(lvl);
    if (lvl >= m_level) {
        append(x);
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
void Logger::append(const T& x)
{
    std::cerr << x;
    if (m_file_stream.is_open()) {
        m_file_stream << x;
    }
}

}; // namespace logger
