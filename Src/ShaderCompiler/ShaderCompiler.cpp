#include "ShaderCompiler.h"

#include "Logger/Logger.h"
#include "glslang/Public/ResourceLimits.h"
#include <format>
#include <fstream>

#include "glslang/MachineIndependent/localintermediate.h"
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"


EShLanguage CastShaderStageToGlslangType(const ShaderCompiler::ShaderStage& stage)
{
    switch (stage)
    {
    case ShaderCompiler::Compute:
        return EShLangCompute;
    case ShaderCompiler::Vertex:
        return EShLangVertex;
    case ShaderCompiler::Fragment:
        return EShLangFragment;
    }
}


void ShaderCompiler::InitGlslCompiler()
{
    glslang::InitializeProcess();
    Logger::Log(Logger::LogLevel::Info, std::format("Shader Compiler Initialized, max version of glsl: [{}]", glslang::GetGlslVersionString()));
}

void ShaderCompiler::ShutdownGlslCompiler()
{
    glslang::FinalizeProcess();
}

/// filePath is from the Shaders folder
ShaderCompiler::ShaderCompilationResult ShaderCompiler::CompileGlslFileIntoSpirV(const std::string& filePath, const ShaderStage& stage)
{
    // Read the code from file
    std::string totalFilePath = ASSETS_DIR "Shaders/" + filePath;

    auto indexOfLastSlash = filePath.find_last_of('/');
    indexOfLastSlash = indexOfLastSlash == std::string::npos ? 0 : indexOfLastSlash + 1;
    std::string fileName = filePath.substr(indexOfLastSlash, filePath.size() - indexOfLastSlash);
    
    std::ifstream file(totalFilePath, std::ifstream::ate | std::ifstream::binary); // is ifstream so always is in "in" mode and ate is "at end"

    if (!file.is_open())
    {
        Logger::Log(Logger::LogLevel::Error, std::format("Failed to open file: [{}]", filePath));
        return ShaderCompiler::ShaderCompilationResult{.bSuccess = false, .errorMessage = (std::format("Failed to open file: [{}]", filePath)) };
    }

    size_t fileSize = file.tellg();

    char* fileData = new char[fileSize + 1]; // Space for the \0
    
    file.seekg(0, std::ifstream::beg);

    file.read(fileData, fileSize);

    file.close();

    fileData[fileSize] = '\0'; // This is because glslang setStrings expects the strings to finish if not it reads trash until parse fail
    
    // Compile
    ShaderCompiler::ShaderCompilationResult result = CompileGlslCodeIntoSpirV(fileData, stage);
    delete[] fileData;
    return result;
}

ShaderCompiler::ShaderCompilationResult ShaderCompiler::CompileGlslCodeIntoSpirV(const std::string& source, const ShaderStage& stage)
{
    ShaderCompiler::ShaderCompilationResult result;
    result.bSuccess = false;
    result.errorMessage = "";
    
    EShLanguage shStage = CastShaderStageToGlslangType(stage);
    glslang::TShader shader(shStage);

    // Must be passed like this because the SetStrings is a const char* const*
    const char* shaderCode = source.c_str();
    
    shader.setStrings(&shaderCode, 1);

    shader.setEnvInput(glslang::EShSourceGlsl, shStage, glslang::EShClientVulkan, 100);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

    shader.parse(
        GetDefaultResources(),
        100,
        false,
        EShMsgDefault
    );

    Logger::Log(Logger::LogLevel::Info, std::format("Parsing shader: {}", shader.getInfoLog()));
    
    glslang::TProgram program;
    program.addShader(&shader);
    program.link(EShMsgDefault);

    Logger::Log(Logger::LogLevel::Info, std::format("Linking program: {}", program.getInfoLog()));

    glslang::TIntermediate* intermediate = program.getIntermediate(shStage);
    glslang::GlslangToSpv(*intermediate, result.spirV);

    if (intermediate->getNumErrors() == 0) result.bSuccess = true;
    
    result.errorMessage += shader.getInfoLog();
    result.errorMessage += program.getInfoLog();
    
    return result;
}



