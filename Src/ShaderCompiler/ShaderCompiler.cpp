#include "ShaderCompiler.h"

#include <format>
#include <fstream>
#include <sstream>
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>

#include "Logger/Logger.h"

/// filePath is from the Shaders folder
std::vector<uint32_t> ShaderCompiler::CompileGlslFile(const std::string& filePath, CompilationFlags flags)
{
    // Read the code from file
    std::string totalFilePath = "../../Assets/Shaders/" + filePath;

    auto indexOfLastSlash = filePath.find_last_of('/');
    indexOfLastSlash = indexOfLastSlash == std::string::npos ? 0 : indexOfLastSlash + 1;
    std::string fileName = filePath.substr(indexOfLastSlash, filePath.size() - indexOfLastSlash);
    
    std::ifstream file(totalFilePath,  std::ios::binary);

    if (!file.is_open())
    {
        return std::vector<uint32_t>();
    }

    file.seekg(0, std::ifstream::end);
    size_t fileSize = file.tellg();

    char* fileData = new char[fileSize];
    
    file.seekg(0, std::ifstream::beg);

    file.read(fileData, fileSize);

    file.close();

    // Compile
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    if (flags & OptimizePerformance) options.SetOptimizationLevel(shaderc_optimization_level_performance);
    if (flags & OptimizeSize) options.SetOptimizationLevel(shaderc_optimization_level_size);
    if (flags & NoOptimize) options.SetOptimizationLevel(shaderc_optimization_level_zero);
    
    shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(fileData, fileSize, shaderc_glsl_compute_shader, fileName.c_str(), options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        Logger::Log(Logger::LogLevel::Error, std::format("The Shader {} have errors: {}\n", fileName, module.GetErrorMessage()));
        return std::vector<uint32_t>();
    }

    return {module.cbegin(), module.cend()};
}
