#include "FGSIncluder.h"

#include "Logger/Logger.h"
#include "Utils/FileUtils.h"

glslang::TShader::Includer::IncludeResult* FGSIncluder::includeSystem(const char* headerName, const char* includeName, size_t inclusionDepth)
{
    std::string totalFilePath = ASSETS_DIR "Shaders/";
    totalFilePath += headerName;

    char* fileData = nullptr;
    size_t fileSize;
    FileUtils::ReadFile(totalFilePath, &fileData, fileSize);
    // Note that the readFile function reserve size for the fileData variable so in this case must clear it in the releaseInclude function
    // no need to store the references because glslang will pass the result that have this data when is ready for be cleared so just clear both the data and the result on the release method
    
    return new IncludeResult(totalFilePath, fileData, sizeof(char) * (fileSize - 1), nullptr);
}

glslang::TShader::Includer::IncludeResult* FGSIncluder::includeLocal(const char* string, const char* text, size_t size)
{
    // TODO: Implement this if want local paths / when this return nullptr for his failure or not implementation calls the includeSystem method automatically
    return nullptr;
}

void FGSIncluder::releaseInclude(IncludeResult* result)
{
    if (result == nullptr) return;
    delete[] result->headerData;
    delete result;
}
