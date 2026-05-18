#include "TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"


/// The filePath is From Assets/Textures
bool TextureLoader::ReadTextureFromFile(const std::string& filePath, TextureResult* outResult)
{
    std::string totalFilePath = ASSETS_DIR "Textures/" + filePath;

    
    int n;
    outResult->data = stbi_load(totalFilePath.c_str(), &outResult->width, &outResult->height, &n, 4);
    
    if (!outResult->data)
    {
        outResult->errorMessage = "Failed to load image with path: " + totalFilePath;
        return false;
    }

    outResult->errorMessage = "Successfully loaded image with path: " + totalFilePath;
    return true;
}

void TextureLoader::FreeTextureResult(TextureResult* result)
{
    stbi_image_free(result->data);
}
