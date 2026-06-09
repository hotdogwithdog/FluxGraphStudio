#include "RGNodeInfo.h"

Graph::EShaderParameterType Graph::CastStringToShaderParameterType(const std::string& str)
{
    if (str == "Float") return Graph::EShaderParameterType::Float;
    if (str == "Int") return Graph::EShaderParameterType::Int;
    if (str == "Image") return Graph::EShaderParameterType::Image;
    if (str == "Vec2") return Graph::EShaderParameterType::Vec2;
    if (str == "Vec3") return Graph::EShaderParameterType::Vec3;
    if (str == "Vec4") return Graph::EShaderParameterType::Vec4;
    if (str == "IVec2") return Graph::EShaderParameterType::IVec2;
    return Graph::EShaderParameterType::None;
}

std::string Graph::CastShaderParameterTypeToString(const EShaderParameterType& type)
{
    switch (type)
    {
        case EShaderParameterType::Float: return "Float";
        case EShaderParameterType::Int: return "Int";
        case EShaderParameterType::Image: return "Image";
        case EShaderParameterType::Vec2: return "Vec2";
        case EShaderParameterType::Vec3: return "Vec3";
        case EShaderParameterType::Vec4: return "Vec4";
        case EShaderParameterType::IVec2: return "IVec2";
        default: return "None";
    }
}

Graph::EImageFormat Graph::CastStringToImageFormat(const std::string& str)
{
    if (str == "RGBA16F") return Graph::EImageFormat::RGBA16F;
    if (str == "RGBA32F") return Graph::EImageFormat::RGBA32F;
    if (str == "RGBA8") return Graph::EImageFormat::RGBA8;
    if (str == "R8") return Graph::EImageFormat::R8;
    if (str == "R16F") return Graph::EImageFormat::R16F;
    if (str == "RG16F") return Graph::EImageFormat::RG16F;
    return Graph::EImageFormat::None;
}

std::string Graph::CastImageFormatToString(const EImageFormat& format)
{
    switch (format)
    {
        case EImageFormat::RGBA16F: return "RGBA16F";
        case EImageFormat::RGBA32F: return "RGBA32F";
        case EImageFormat::RGBA8: return "RGBA8";
        case EImageFormat::R8: return "R8";
        case EImageFormat::R16F: return "R16F";
        case EImageFormat::RG16F: return "RG16F";
        default: return "None";
    }
}

RGNodeInfo::RGNodeInfo(std::filesystem::path path)
{
    SetPathFile(path);
}

RGNodeInfo::RGNodeInfo(const RGNodeInfo& other)
{
    _nodeFileName = other._nodeFileName;
    inputs.reserve(other.inputs.size());
    for (Graph::ShaderParameter parameter : other.inputs)
    {
        inputs.push_back(parameter);
    }
    uniforms.reserve(other.uniforms.size());
    for (Graph::ShaderParameter parameter : other.uniforms)
    {
        uniforms.push_back(parameter);
    }
    outputs.reserve(other.outputs.size());
    for (Graph::ShaderParameter parameter : other.outputs)
    {
        outputs.push_back(parameter);
    }
}

void RGNodeInfo::SetPathFile(std::filesystem::path path)
{
    _nodeFileName = path;
}

std::string RGNodeInfo::GetName()
{
    return _nodeFileName.filename().string();
}
