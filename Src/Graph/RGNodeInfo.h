#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "NodesResourceManager.h"
#include "Assets/AssetsManager.h"
#include "Utils/AssetsUtils.h"
#include "GPU/VulkanDescriptors.h"

namespace Graph
{
    enum class EShaderParameterType
    {
        None = 0,
        Bool,
        Float,
        Int,
        Image,
        Vec2,
        Vec3,
        Vec4,
        IVec2,
        IVec3,
        IVec4
    };

    EShaderParameterType CastStringToShaderParameterType(const std::string& str);
    std::string CastShaderParameterTypeToString(const EShaderParameterType& type);
    std::string CastShaderParameterTypeToGlslTypeString(const EShaderParameterType& type, const bool bIsStorageImage = false);
    uint32_t GetSizeOfType(EShaderParameterType type);

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
    std::string CastImageFormatToGlslFormatString(const EImageFormat& format);
    std::string CastImageFormatToString(const EImageFormat& format);

    struct ShaderParameter
    {
        EShaderParameterType type;
        EImageFormat format;
        std::string name;
    };

    struct ShaderParameterBindingInfo
    {
        uint32_t binding;
        EShaderParameterType type;
        std::string name;
    };

    struct UniformShaderParameterBindingInfo
    {
        EShaderParameterType type;
        size_t parameterSize;
        size_t offsetInBuffer;
        std::string name;
    };

    struct ParameterBindingLayout
    {
        std::vector<ShaderParameterBindingInfo> inputBindings;
        std::vector<ShaderParameterBindingInfo> outputBindings;
        std::vector<ShaderParameterBindingInfo> uniformImageBindings;
        std::vector<UniformShaderParameterBindingInfo> uniformParameterBindings;
        uint32_t uniformBufferBinding;
        uint32_t uniformBufferSize;

        void ClearAll()
        {
            inputBindings.clear();
            outputBindings.clear();
            uniformImageBindings.clear();
            uniformParameterBindings.clear();
        }
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

        std::ostream& operator<<(std::ostream& os) const
        {
            return os << ToString();
        }
    };
}

// The Parameters will be uploaded on a big Uniform buffer but the images (input, output, uniforms)
// will be uploaded in a different set, also the size of the images will be uploaded on a different uniform buffer
// Class that has the info of Inputs, Outputs, Uniforms of a node (The Types of them)
// This is get from the .gNode file of the node
class RGNodeInfo
{
private:
    Graph::ParameterBindingLayout _parameterBindingLayout;
    
public:
    RGNodeInfo() = default;
    RGNodeInfo(const RGNodeInfo& other);

    std::string name;
    std::string sourceCode;
    std::string finalCode;
    uint32_t version = 1;
    uint64_t hash;
    
    std::vector<Graph::ShaderParameter> inputs;
    std::vector<Graph::ShaderParameter> outputs;
    std::vector<Graph::ShaderParameter> uniforms;

    void GetPoolSizes(std::vector<Descriptors::PoolSizeRatio>& outSizes) const;
    Graph::ParameterBindingLayout const * GetParameterBindingLayout() const;
    void FillDescriptorSetLayoutBuilder(Descriptors::DescriptorLayoutBuilder& outBuilder);
    bool GenerateFinalCode();
    
private:
    std::string GenerateBindings() const;
};
