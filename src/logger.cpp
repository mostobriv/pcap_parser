#include "logger.h"

#include <stdexcept>


namespace logger
{

std::mutex __BaseLoggerLock::logger_mutex;

Logger<LVL_DEBUG> logger ("global");

} // namespace logger
