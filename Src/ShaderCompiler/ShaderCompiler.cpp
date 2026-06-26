#include "ShaderCompiler.h"

#include "FGSIncluder.h"
#include "Logger/Logger.h"
#include "SPIRV/GlslangToSpv.h"
#include "Utils/FileUtils.h"
#include "glslang/MachineIndependent/localintermediate.h"
#include "glslang/Public/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"
#include <format>

namespace ShaderCompiler
{
    FGSIncluder* _includer = nullptr;
}

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


void ShaderCompiler::Init()
{
    glslang::InitializeProcess();

    if (_includer == nullptr) _includer = new FGSIncluder();
    else Logger::Log(Logger::LogLevel::Warning, "ShaderCompiler::Init: The FGSIncluder has been previously initialized, using that instead of create another");
    
    Logger::Log(Logger::LogLevel::Info, std::format("Shader Compiler Initialized, max version of glsl: [{}]", glslang::GetGlslVersionString()));
}

void ShaderCompiler::Shutdown()
{
    glslang::FinalizeProcess();

    delete _includer;
}

/// filePath is from the Shaders folder
ShaderCompiler::ShaderCompilationResult ShaderCompiler::CompileGlslFileIntoSpirV(const std::string& filePath, const ShaderStage& stage)
{
    // Read the code from file
    std::string totalFilePath = ASSETS_DIR "Shaders/" + filePath;
    
    char* fileData = nullptr;
    size_t fileSize;
    FileUtils::ReadFile(totalFilePath, &fileData, fileSize);
    
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

    shader.setPreamble("#extension GL_GOOGLE_include_directive : require\n");

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
        EShMsgDefault,
        *_includer
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



