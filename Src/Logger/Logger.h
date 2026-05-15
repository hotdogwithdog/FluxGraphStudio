#pragma once
#include <array>
#include <string>

namespace Logger
{
    #define LOG_MESSAGE_MAX 1000
    
    enum class LogLevel
    {
        None = 0,
        Debug,
        Warning,
        Error
    };
    
    struct LogMessage
    {
        LogLevel level;
        std::string message;
        LogMessage()
        {
            level = LogLevel::None;
            message = "";
        }
            
        LogMessage(const LogLevel& level, const std::string& message) : level(level), message(message) { }
    };

    void Log(const LogLevel& level, const std::string& message);
    
}

