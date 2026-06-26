#pragma once
#include <string>

namespace FileUtils
{
    /// The function reads a file put his content on outData and his size on outSize it skips the UTF8BOM if exist
    /// The outDataSize is the size of the buffer outData including the \0 character at the end
    /// IMPORTANT: Do a memory reserve for the data, the user must manage it
    void ReadFile(const std::string& filePath, char** outData, size_t& outDataSize);
}
