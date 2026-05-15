#include "Logger.h"

#include <iostream>
#include <sstream>

#define SET_COLOR_CONSOLE(value) "\033[38;5;" #value "m"
#define RESET_COLOR_CONSOLE "\033[0m"

namespace Logger
{
    int messagesCount = 0;
    std::array<LogMessage, LOG_MESSAGE_MAX> messages;
}

void Logger::Log(const LogLevel& level, const std::string& message)
{
    std::ostringstream headerLog;
    switch (level)
    {
    case LogLevel::Debug:
        headerLog << "DEBUG: ";
        break;
    case LogLevel::Warning:
        headerLog << SET_COLOR_CONSOLE(3) << "WARNING: " << RESET_COLOR_CONSOLE;
        break;
    case LogLevel::Error:
        headerLog << SET_COLOR_CONSOLE(1) << "ERROR: " << RESET_COLOR_CONSOLE;
        break;
    default:
        std::cout << SET_COLOR_CONSOLE(196) << "UNKOWN LOG LEVEL: " << RESET_COLOR_CONSOLE << message << std::endl;
        return;
    }

    std::cout << headerLog.str() << message << std::endl;

    messages[messagesCount] = LogMessage(level, message);
}
