#pragma once

#include <string>
#include <vector>

namespace ShaderCompiler
{
    enum ShaderStage
    {
        None = 0,
        Compute,
        Vertex,
        Fragment
    };

    struct ShaderCompilationResult
    {
        bool bSuccess;
        std::vector<uint32_t> spirV;
        std::string errorMessage;
    };

    void InitGlslCompiler();

    void ShutdownGlslCompiler();


    /// The Compiler must be initialized
    /// FilePath From Assets/Shaders so if you have Assets/Shaders/Common/common.glsl the path is just Common/common.glsl
    ShaderCompilationResult CompileGlslFileIntoSpirV(const std::string& filePath, const ShaderStage& stage = ShaderStage::Compute);

    ShaderCompilationResult CompileGlslCodeIntoSpirV(const std::string& source, const ShaderStage& stage = ShaderStage::Compute);
}
