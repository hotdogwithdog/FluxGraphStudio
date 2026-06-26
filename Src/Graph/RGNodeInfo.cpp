#include "RGNodeInfo.h"

#include <imgui.h>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"


Graph::EShaderParameterType Graph::CastStringToShaderParameterType(const std::string& str)
{
    if (str == "Bool") return Graph::EShaderParameterType::Bool;
    if (str == "Float") return Graph::EShaderParameterType::Float;
    if (str == "Int") return Graph::EShaderParameterType::Int;
    if (str == "Image") return Graph::EShaderParameterType::Image;
    if (str == "Vec2") return Graph::EShaderParameterType::Vec2;
    if (str == "Vec3") return Graph::EShaderParameterType::Vec3;
    if (str == "Vec4") return Graph::EShaderParameterType::Vec4;
    if (str == "IVec2") return Graph::EShaderParameterType::IVec2;
    if (str == "IVec3") return Graph::EShaderParameterType::IVec3;
    if (str == "IVec4") return Graph::EShaderParameterType::IVec4;
    return Graph::EShaderParameterType::None;
}

std::string Graph::CastShaderParameterTypeToString(const EShaderParameterType& type)
{
    switch (type)
    {
        case EShaderParameterType::Bool: return "Bool";
        case EShaderParameterType::Float: return "Float";
        case EShaderParameterType::Int: return "Int";
        case EShaderParameterType::Image: return "Image";
        case EShaderParameterType::Vec2: return "Vec2";
        case EShaderParameterType::Vec3: return "Vec3";
        case EShaderParameterType::Vec4: return "Vec4";
        case EShaderParameterType::IVec2: return "IVec2";
        case EShaderParameterType::IVec3: return "IVec3";
        case EShaderParameterType::IVec4: return "IVec4";
        default: return "None";
    }
}

std::string Graph::CastShaderParameterTypeToGlslTypeString(const EShaderParameterType& type, const bool bIsStorageImage /* = false */)
{
    switch (type)
    {
        case EShaderParameterType::Bool: return "bool";
        case EShaderParameterType::Float: return "float";
        case EShaderParameterType::Int: return "int";
        case EShaderParameterType::Image:
            {
                if (bIsStorageImage) return "image2D";
                return "sampler2D";
            }
        case EShaderParameterType::Vec2: return "vec2";
        case EShaderParameterType::Vec3: return "vec3";
        case EShaderParameterType::Vec4: return "vec4";
        case EShaderParameterType::IVec2: return "ivec2";
        case EShaderParameterType::IVec3: return "ivec3";
        case EShaderParameterType::IVec4: return "ivec4";
        default: return "None";
    }
}

uint32_t Graph::GetSizeOfType(EShaderParameterType type)
{
    switch (type)
    {
        case EShaderParameterType::Bool: return sizeof(bool);
        case EShaderParameterType::Float: return sizeof(float);
        case EShaderParameterType::Int: return sizeof(int);
        case EShaderParameterType::Vec2: return sizeof(glm::vec2);
        case EShaderParameterType::Vec3: return sizeof(glm::vec3);
        case EShaderParameterType::Vec4: return sizeof(glm::vec4);
        case EShaderParameterType::IVec2: return sizeof(glm::ivec2);
        case EShaderParameterType::IVec3: return sizeof(glm::ivec3);
        case EShaderParameterType::IVec4: return sizeof(glm::ivec4);
        default:
            {
                Logger::Log(Logger::LogLevel::Warning, std::format("Unknown Shader Parameter Type: {}", Graph::CastShaderParameterTypeToString(type)));
                return 0;
            }
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

std::string Graph::CastImageFormatToGlslFormatString(const EImageFormat& format)
{
    switch (format)
    {
        case EImageFormat::RGBA16F: return "rgba16f";
        case EImageFormat::RGBA32F: return "rgba32f";
        case EImageFormat::RGBA8: return "rgba8";
        case EImageFormat::R8: return "r8";
        case EImageFormat::R16F: return "r16";
        case EImageFormat::RG16F: return "rg16";
        default: return "None";
    }
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

RGNodeInfo::RGNodeInfo(const RGNodeInfo& other)
{
    name = other.name;
    sourceCode = other.sourceCode;
    finalCode = other.finalCode;
    version = other.version;
    hash = other.hash;
    _parameterBindingLayout = other._parameterBindingLayout; // Should be a deep copy because the default use the operator= on his elements, and they are vectors that do a deep copy

    // They are deep copies
    inputs = other.inputs;
    outputs = other.outputs;
    uniforms = other.uniforms;
    
    // inputs.reserve(other.inputs.size());
    // for (Graph::ShaderParameter parameter : other.inputs)
    // {
    //     inputs.push_back(parameter);
    // }
    // uniforms.reserve(other.uniforms.size());
    // for (Graph::ShaderParameter parameter : other.uniforms)
    // {
    //     uniforms.push_back(parameter);
    // }
    // outputs.reserve(other.outputs.size());
    // for (Graph::ShaderParameter parameter : other.outputs)
    // {
    //     outputs.push_back(parameter);
    // }
}

void RGNodeInfo::GetPoolSizes(std::vector<Descriptors::PoolSizeRatio>& outSizes) const
{
    int reserveCount = 1; // The uniform buffer is always been added (for at least the metadata of the outputImage (forced to be at least one output image to be a valid node))
    uint32_t imageCount = inputs.size();
    if (imageCount > 0) reserveCount++;
    for (size_t i = 0; i < uniforms.size(); ++i)
    {
        if (uniforms[i].type == Graph::EShaderParameterType::Image) imageCount++;
    }

    if (outputs.size() > 0) reserveCount++;
    
    outSizes.reserve(reserveCount);

    if (imageCount > 0)
    {
        outSizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageCount});
    }
    
    outSizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1});
    
    if (outputs.size() > 0)
    {
        outSizes.push_back({VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, (uint32_t)outputs.size()});
    }
}

Graph::ParameterBindingLayout const *  RGNodeInfo::GetParameterBindingLayout() const
{
    return &_parameterBindingLayout;
}

void RGNodeInfo::FillDescriptorSetLayoutBuilder(Descriptors::DescriptorLayoutBuilder& outBuilder)
{
    outBuilder.Clear();
    _parameterBindingLayout.ClearAll();

    uint32_t imageCount = 0;
    uint32_t bindingIndex = 0;
    imageCount += inputs.size();
    _parameterBindingLayout.inputBindings.reserve(inputs.size());
    for (size_t i = 0; i < inputs.size(); ++i)
    {
        outBuilder.AddBinding(i + bindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        _parameterBindingLayout.inputBindings.emplace_back(i + bindingIndex, inputs[i].type, inputs[i].name);
    }
    bindingIndex += inputs.size();
    
    int uniformImagesCount = 0;
    size_t currentBufferOffset = 0;
    for (size_t i = 0; i < uniforms.size(); ++i)
    {
        if (uniforms[i].type == Graph::EShaderParameterType::Image)
        {
            outBuilder.AddBinding(uniformImagesCount + bindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            _parameterBindingLayout.uniformImageBindings.emplace_back(uniformImagesCount + bindingIndex, uniforms[i].type, uniforms[i].name);
            uniformImagesCount++;
        }
        else
        {
            size_t parameterSize = Graph::GetSizeOfType(uniforms[i].type);
            _parameterBindingLayout.uniformParameterBindings.emplace_back(uniforms[i].type, parameterSize, currentBufferOffset + parameterSize, uniforms[i].name);
            currentBufferOffset += parameterSize;
        }
    }
    bindingIndex += uniformImagesCount;
    imageCount += uniformImagesCount;
    
    outBuilder.AddBinding(bindingIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    _parameterBindingLayout.uniformBufferBinding = bindingIndex;

    bindingIndex++;
    _parameterBindingLayout.outputBindings.reserve(outputs.size());
    for (size_t i = 0; i < outputs.size(); ++i)
    {
        outBuilder.AddBinding(i + bindingIndex, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        _parameterBindingLayout.outputBindings.emplace_back(i + bindingIndex, outputs[i].type, outputs[i].name);
    }

    // Images metadata they go on the same uniform buffer so just increment it, they are IVec2
    imageCount += outputs.size();

    // Maybe change to be inside the other loops that are before but like that is more clean
    std::vector<std::string> imageMetadataNames;
    imageMetadataNames.reserve(imageCount);
    for (size_t i = 0; i < inputs.size(); ++i)
    {
        imageMetadataNames.push_back(inputs[i].name + "_Size");
    }
    for (size_t i = 0; i < uniforms.size(); ++i)
    {
        if (uniforms[i].type == Graph::EShaderParameterType::Image)
        {
            imageMetadataNames.push_back(uniforms[i].name + "_Size");
        }
    }
    for (size_t i = 0; i < outputs.size(); ++i)
    {
        imageMetadataNames.push_back(outputs[i].name + "_Size");
    }
    

    size_t parameterSize = Graph::GetSizeOfType(Graph::EShaderParameterType::IVec2);
    _parameterBindingLayout.uniformParameterBindings.reserve(_parameterBindingLayout.uniformParameterBindings.size() + imageCount);
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        _parameterBindingLayout.uniformParameterBindings.emplace_back(Graph::EShaderParameterType::IVec2, parameterSize, currentBufferOffset + parameterSize, imageMetadataNames[i]);
        currentBufferOffset += parameterSize;
    }
    _parameterBindingLayout.uniformBufferSize = currentBufferOffset; // Total size of the uniform buffer counting the metadata of images
}

std::string RGNodeInfo::GenerateBindings() const
{
    if (_parameterBindingLayout.inputBindings.size() != inputs.size() &&
        _parameterBindingLayout.outputBindings.size() != outputs.size())
    {
        Logger::Log(Logger::LogLevel::Error, "RGNodeInfo::GenerateBindings: Number of inputs or outputs not match probably this was call before the FillDescriptorSetLayoutBuilder method");
        return "";
    }

    
    std::ostringstream generatedBindings;
    
    //TODO: Change this to be dinamic right now i will hardcode the 16x16 workgroups
    generatedBindings << "\n/////////////////////////////// GENERATED WORKGROUP\n";
    generatedBindings << "\nlayout (local_size_x = 16, local_size_y = 16) in;\n";
    generatedBindings << "\n/////////////////////////////// END OF GENERATED WORKGROUP\n";

    generatedBindings << "\n/////////////////////////////// GENERATED BINDINGS\n";
    generatedBindings << "\n// Generated Bindings Inputs\n";
    for (Graph::ShaderParameterBindingInfo const & bindingInfo : _parameterBindingLayout.inputBindings)
    {
        generatedBindings << "layout(set = 1, binding = " << bindingInfo.binding << ") uniform " <<
            Graph::CastShaderParameterTypeToGlslTypeString(bindingInfo.type, false) << " " << bindingInfo.name << ";\n";
    }

    generatedBindings << "\n\n// Generated Bindings Uniforms Images\n";
    for (Graph::ShaderParameterBindingInfo const & bindingInfo : _parameterBindingLayout.uniformImageBindings)
    {
        generatedBindings << "layout(set = 1, binding = " << bindingInfo.binding << ") uniform " <<
            Graph::CastShaderParameterTypeToGlslTypeString(bindingInfo.type, false) << " " << bindingInfo.name << ";\n";
    }

    generatedBindings << "\n\n// Generated Bindings Uniform Buffer\n";
    generatedBindings << "layout(set = 1, binding = " << _parameterBindingLayout.uniformBufferBinding << ") uniform UserUniformBuffer\n{\n";
    for (Graph::UniformShaderParameterBindingInfo const & bindingInfo : _parameterBindingLayout.uniformParameterBindings)
    {
        // Is generated in order so just put the parameters in order (not the best for alignment in some cases but this can be tweek by the user with the order of declaration of the parameters)
        generatedBindings << "\t" << Graph::CastShaderParameterTypeToGlslTypeString(bindingInfo.type) << " " << bindingInfo.name << ";\n";
    }
    generatedBindings << "} UB;\n";

    generatedBindings << "\n\n// Generated Bindings Outputs\n";
    for (size_t i = 0; i < _parameterBindingLayout.outputBindings.size(); ++i)
    {
        if (_parameterBindingLayout.outputBindings[i].name != outputs[i].name)
        {
            Logger::Log(Logger::LogLevel::Warning, std::format("RGNodeInfo::GenerateBindings: The order of the outputs bindings do not match with the order of the outputs parsed, skipping this index: {}", i));
            continue;
        }
        generatedBindings << "layout(" << Graph::CastImageFormatToGlslFormatString(outputs[i].format) << ", set = 1, binding = " <<
            _parameterBindingLayout.outputBindings[i].binding << ") uniform " <<
            Graph::CastShaderParameterTypeToGlslTypeString(_parameterBindingLayout.outputBindings[i].type, true) <<
                " " << _parameterBindingLayout.outputBindings[i].name << ";\n";
    }

    generatedBindings << "\n/////////////////////////////// END OF GENERATED BINDINGS\n";
    
    return generatedBindings.str();
}

bool RGNodeInfo::GenerateFinalCode()
{
    if (_parameterBindingLayout.outputBindings.size() <= 0) return false;

    finalCode.clear();
    finalCode.reserve(sourceCode.size()); // At least this will be allocated so use less space
    finalCode += "#version 460\n";
    finalCode += GenerateBindings();
    finalCode += "\n#include <FGSCommons.glsl>\n";
    finalCode += "\n#line 1 \"UserShader\"\n";
    finalCode += sourceCode;
    return true;
}
















