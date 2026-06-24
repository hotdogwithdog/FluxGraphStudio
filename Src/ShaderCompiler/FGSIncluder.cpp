#include "FGSIncluder.h"

#include <fstream>

#include "Logger/Logger.h"

glslang::TShader::Includer::IncludeResult* FGSIncluder::includeSystem(const char* headerName, const char* includeName, size_t inclusionDepth)
{
    std::string totalFilePath = ASSETS_DIR "Shaders/";
    totalFilePath += headerName;

    std::ifstream file(totalFilePath, std::ifstream::ate | std::ifstream::binary); // is ifstream so always is in "in" mode and ate is "at end"

    if (!file.is_open())
    {
        Logger::Log(Logger::LogLevel::Error, std::format("Failed to open file: [{}]", totalFilePath));
        return nullptr;
    }

    size_t fileSize = file.tellg();

    char* fileData = new char[fileSize + 1]; // Space for the \0
    
    file.seekg(0, std::ifstream::beg);

    file.read(fileData, fileSize);

    file.close();

    fileData[fileSize] = '\0';
    
    return new IncludeResult(totalFilePath, fileData, sizeof(char) * (fileSize + 1), nullptr);
}

glslang::TShader::Includer::IncludeResult* FGSIncluder::includeLocal(const char* string, const char* text, size_t size)
{
    // TODO: Implement this if want local paths
    return nullptr;
}

void FGSIncluder::releaseInclude(IncludeResult* result)
{
    delete result->headerData;
    delete result;
}
