#include "FileUtils.h"

#include "Logger/Logger.h"
#include <fstream>

void FileUtils::ReadFile(const std::string& filePath, char** outData, size_t& outDataSize)
{
    std::ifstream file(filePath, std::ifstream::ate | std::ifstream::binary); // is ifstream so always is in "in" mode and ate is "at end"

    if (!file.is_open())
    {
        Logger::Log(Logger::LogLevel::Error, std::format("Failed to open file: [{}]", filePath));
        return;
    }

    outDataSize = file.tellg();

    file.seekg(0, std::ifstream::beg);
    
    char a = file.get();
    char b = file.get();
    char c = file.get();
    if (a != (char)0xEF || b != (char)0xBB || c != (char)0xBF) file.seekg(0, std::ifstream::beg);
    else
    {
        Logger::Log(Logger::LogLevel::Warning, std::format("The file have UTF8BOM File: [{}]", filePath));
        outDataSize -= 3;
    }

    *outData = new char[outDataSize + 1]; // Space for the \0

    file.read(*outData, outDataSize);

    file.close();

    (*outData)[outDataSize] = '\0'; // This is because glslang setStrings expects the strings to finish if not it reads trash until parse fail
    outDataSize += 1; // The \0 char
}
