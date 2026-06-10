#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "NodesResourceManager.h"
#include "Assets/AssetsManager.h"
#include "Utils/AssetsUtils.h"

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
    std::string CastShaderParameterTypeToString(const EShaderParameterType& type);

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
    std::string CastImageFormatToString(const EImageFormat& format);

    struct ShaderParameter
    {
        EShaderParameterType type;
        EImageFormat format;
        std::string name;
    };


    struct ConnectionInfo
    {
        DescriptionNodeInstanceHandle originNodeInstance = 0;
        int originIndex = -1;
        DescriptionNodeInstanceHandle destinationNodeInstance = 0;
        int destinationIndex = -1;
        bool bIsStart = false;
        bool bIsEnd = false;
        
        bool IsValid() const
        {
            if (bIsStart && bIsEnd) return false;
            if (bIsStart) return AssetsUtils::IsValid(destinationNodeInstance) && destinationIndex >= 0;
            if (bIsEnd) return AssetsUtils::IsValid(originNodeInstance) && originIndex >= 0;
            
            return AssetsUtils::IsValid(originNodeInstance) && originIndex >= 0 && AssetsUtils::IsValid(destinationNodeInstance) && destinationIndex >= 0;
        }

        std::string ToString() const
        {
            std::ostringstream out;
            out << "origin: {" << originNodeInstance << ", " << originIndex <<
                "}, destination: {" << destinationNodeInstance << ", " << destinationIndex << "}, isStart = " << bIsStart << ", isEnd = " << bIsEnd << std::endl;
            return out.str();
        }
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
    RGNodeInfo(const RGNodeInfo& other);

    std::string name;
    std::string sourceCode;
    uint32_t version = 1;
    uint64_t hash;
    
    std::vector<Graph::ShaderParameter> inputs;
    std::vector<Graph::ShaderParameter> outputs;
    std::vector<Graph::ShaderParameter> uniforms;
};
