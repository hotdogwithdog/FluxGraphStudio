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

RGNodeInfo::RGNodeInfo(std::filesystem::path path)
{
    SetPathFile(path);
}

void RGNodeInfo::SetPathFile(std::filesystem::path path)
{
    _nodeFileName = path;
}

std::string RGNodeInfo::GetName()
{
    return _nodeFileName.filename().string();
}
