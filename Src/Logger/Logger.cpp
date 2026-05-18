#include "Logger.h"

#include <iostream>
#include <sstream>

#define STRINGIFY_HELPER(X) #X
#define STRINGIFY(X) STRINGIFY_HELPER(X)


#define SET_COLOR_CONSOLE_WITH_MACRO(X) "\033[38;5;" STRINGIFY(X) "m"
#define SET_COLOR_CONSOLE(value) "\033[38;5;" #value "m"
#define RESET_COLOR_CONSOLE "\033[0m"

#define ERROR_COLOR_MESSAGE 1
#define WARNING_COLOR_MESSAGE 3
#define INFO_COLOR_MESSAGE 27
#define UNKNOWN_COLOR_MESSAGE 196

#define SET_INFO_COLOR_CONSOLE SET_COLOR_CONSOLE_WITH_MACRO(INFO_COLOR_MESSAGE)
#define SET_ERROR_COLOR_CONSOLE SET_COLOR_CONSOLE_WITH_MACRO(ERROR_COLOR_MESSAGE)
#define SET_WARNING_COLOR_CONSOLE SET_COLOR_CONSOLE_WITH_MACRO(WARNING_COLOR_MESSAGE)
#define SET_UNKNOWN_COLOR_CONSOLE SET_COLOR_CONSOLE_WITH_MACRO(UNKNOWN_COLOR_MESSAGE)

namespace Logger
{
    int endMessagesIndex = 0;
    int startMessageIndex = 0;
    std::array<LogMessage, LOG_MESSAGE_MAX> messages;
}

void Logger::Log(const LogLevel& level, const std::string& message)
{
    std::ostringstream headerLog;
    switch (level)
    {
    case LogLevel::Info:
        headerLog << SET_INFO_COLOR_CONSOLE << "INFO: " << RESET_COLOR_CONSOLE;
        break;
    case LogLevel::Debug:
        headerLog << "DEBUG: ";
        break;
    case LogLevel::Warning:
        headerLog << SET_WARNING_COLOR_CONSOLE << "WARNING: " << RESET_COLOR_CONSOLE;
        break;
    case LogLevel::Error:
        headerLog << SET_ERROR_COLOR_CONSOLE << "ERROR: " << RESET_COLOR_CONSOLE;
        break;
    default:
        std::cout << SET_UNKNOWN_COLOR_CONSOLE << "UNKOWN LOG LEVEL: " << RESET_COLOR_CONSOLE << message << std::endl;
        return;
    }

#if _DEBUG
    std::cout << headerLog.str() << message << std::endl;
#endif
    
    messages[endMessagesIndex] = LogMessage(level, message);
    endMessagesIndex = (endMessagesIndex + 1) % LOG_MESSAGE_MAX;
    if (startMessageIndex == endMessagesIndex) startMessageIndex = (startMessageIndex + 1) % LOG_MESSAGE_MAX;
}