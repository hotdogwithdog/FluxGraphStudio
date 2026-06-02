#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace Graph
{
    enum class EShaderParameterType
    {
        None = 0,
        Float,
        Int,
        Image,
        Vec2,
        Vec3,
        Vec4,
        IVec2
    };

    EShaderParameterType CastStringToShaderParameterType(const std::string& str);

    enum class EImageFormat
    {
        None = 0,
        RGBA16F,
        RGBA32F,
        RGBA8,
        R8,
        R16F,
        RG16F
    };

    EImageFormat CastStringToImageFormat(const std::string& str);

    struct ShaderParameter
    {
        EShaderParameterType type;
        EImageFormat format;
        std::string name;
    };
}

// The Parameters will be uploaded on a big Uniform buffer but the images (input, output, uniforms)
// will be uploaded in a different set, also the size of the images will be uploaded on a different uniform buffer
// Class that has the info of Inputs, Outputs, Uniforms of a node (The Types of them)
// This is get from the .gNode file of the node
class RGNodeInfo
{
public:
    RGNodeInfo() = default;
    
    std::vector<Graph::ShaderParameter> inputs;
    std::vector<Graph::ShaderParameter> outputs;
    std::vector<Graph::ShaderParameter> uniforms;

public:
    RGNodeInfo(std::filesystem::path path);

    void SetPathFile(std::filesystem::path path);

    std::string GetName();
    
private:
    std::filesystem::path _nodeFileName;
};
