#pragma once

#include <string>



namespace TextureLoader
{
    struct TextureResult
    {
        int width, height;
        std::string errorMessage;
        void* data;
    };

    bool ReadTextureFromFile(const std::string& filePath, TextureResult* outResult);

    void FreeTextureResult(TextureResult* result);
    
}
