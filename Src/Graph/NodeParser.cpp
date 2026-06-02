#include "NodeParser.h"

#include <fstream>

#include "Logger/Logger.h"
#include "Utils/StringUtils.h"

namespace NodeParser
{
    enum EParserSection
    {
        None = 0,
        Inputs,
        Outputs,
        Uniforms,
        Shader
    };
}

void ParseInput(NodeParser::NodeParserResult& result, std::string& line, std::string totalPath, int currentLine);
void ParseOutput(NodeParser::NodeParserResult& result, std::string& line, std::string totalPath, int currentLine);
void ParseUniform(NodeParser::NodeParserResult& result, std::string& line, std::string totalPath, int currentLine);

NodeParser::NodeParserResult NodeParser::CompileGNodeFile(std::filesystem::path path)
{
    NodeParserResult result = {};
    result.nodeInfo.SetPathFile(path);
    result.bSuccess = true;
    result.errorMessage = "";
    if (path.extension() != ".gNode")
    {
        Logger::Log(Logger::LogLevel::Error, std::format("The file of path = {} is not a .gNode file", path.string()));
        result.errorMessage = std::format("The file of path = {} is not a .gNode file", path.string());
        result.bSuccess = false;
        return result;
    }

    std::string totalFilePath = ASSETS_DIR "Nodes/" + path.string();

    std::ifstream file(totalFilePath, std::ifstream::binary);

    if (!file.is_open())
    {
        Logger::Log(Logger::LogLevel::Error, std::format("Failed to open file: [{}]", totalFilePath));
        result.errorMessage = std::format("Failed to open file: [{}]", totalFilePath);
        result.bSuccess = false;
        return result;
    }
    
    EParserSection currentSection = EParserSection::None;
    int currentLine = 0;
    for (std::string line; std::getline(file, line); )
    {
        currentLine++;
        StringUtils::RemoveUTF8BOM(line);
        line = StringUtils::Trim(line);
        if (line.empty()) continue;
        

        if (line[0] == '@')
        {
            if (line == "@inputs")
            {
                currentSection = EParserSection::Inputs;
                continue;
            }
            if (line == "@outputs")
            {
                currentSection = EParserSection::Outputs;
                continue;
            }
            if (line == "@uniforms")
            {
                currentSection = EParserSection::Uniforms;
                continue;
            }
            if (line == "@shader")
            {
                currentSection = EParserSection::Shader;
                // No continue because it will go out of the loop and i do not want to do other iteration for let the file pointer just at the start of the shader code
            }
        }
        
        switch (currentSection)
        {
            case EParserSection::Inputs:
                ParseInput(result, line, totalFilePath, currentLine);
                continue;
            case EParserSection::Outputs:
                ParseOutput(result, line, totalFilePath, currentLine);
                continue;
            case EParserSection::Uniforms:
                ParseUniform(result, line, totalFilePath, currentLine);
                continue;
            default:
                break;
        }
        if (currentSection == EParserSection::None) continue;

        // Just reach if shader It's a little confuse the execution order but if is the shader go out immediatly of the loop
        break;
        
    }

    // Send to the compiler TODO: Maybe this must be separated and the result must be only of the parser things and not also the glsl compilation, but for now
    if (!result.bSuccess)
    {
        file.close();
        Logger::Log(Logger::LogLevel::Error, result.errorMessage);
        return result;
    }
    result.errorMessage = "Parse Success";

    // Get the Shader code // The pointer on the file is advanced to the start of the shader

    
    size_t shaderStart = file.tellg();
    file.seekg(0, std::ios::end);
    size_t shaderEnd = file.tellg();

    size_t shaderSize = shaderEnd - shaderStart;
    
    char* fileData = new char[shaderSize + 1];
    file.seekg(shaderStart, std::ios::beg);

    file.read(fileData, shaderSize);

    file.close();

    fileData[shaderSize] = '\0';

    result.compilationResult = ShaderCompiler::CompileGlslCodeIntoSpirV(fileData, ShaderCompiler::ShaderStage::Compute);
    delete[] fileData;
    return result;
}

void ParseInput(NodeParser::NodeParserResult& result, std::string& line, std::string totalPath, int currentLine)
{
    size_t spacePos = line.find_first_of(' ');
    if (spacePos == std::string::npos)
    {
        result.errorMessage += std::format("Parse error on file [{}]: Expected space not found on line {}", totalPath, currentLine);
        result.bSuccess = false;
        return;
    }
    std::string format = line.substr(0, spacePos);
    std::string name = StringUtils::Trim(line.substr(spacePos + 1, line.length() - spacePos + 1));

    Graph::EImageFormat eFormat = Graph::CastStringToImageFormat(format);
    if (eFormat == Graph::EImageFormat::None)
    {
        result.errorMessage += std::format("Parse error on file [{}]: Unknown image format: [{}]", totalPath, format);
        result.bSuccess = false;
        return;
    }
    
    Graph::ShaderParameter shaderParameter;
    shaderParameter.name = name;
    shaderParameter.format = eFormat;
    shaderParameter.type = Graph::EShaderParameterType::Image;
    
    result.nodeInfo.inputs.push_back(shaderParameter);
}

void ParseOutput(NodeParser::NodeParserResult& result, std::string& line, std::string totalPath, int currentLine)
{
    size_t spacePos = line.find_first_of(' ');
    if (spacePos == std::string::npos)
    {
        result.errorMessage += std::format("Parse error on file [{}]: Expected space not found on line {}", totalPath, currentLine);
        result.bSuccess = false;
        return;
    }
    std::string format = line.substr(0, spacePos);
    std::string name = StringUtils::Trim(line.substr(spacePos + 1, line.length() - spacePos + 1));

    Graph::EImageFormat eFormat = Graph::CastStringToImageFormat(format);
    if (eFormat == Graph::EImageFormat::None)
    {
        result.errorMessage += std::format("Parse error on file [{}]: Unknown image format: [{}]", totalPath, format);
        result.bSuccess = false;
        return;
    }

    Graph::ShaderParameter shaderParameter;
    shaderParameter.name = name;
    shaderParameter.format = eFormat;
    shaderParameter.type = Graph::EShaderParameterType::Image;
    
    result.nodeInfo.outputs.push_back(shaderParameter);
}

void ParseUniform(NodeParser::NodeParserResult& result, std::string& line, std::string totalPath, int currentLine)
{
    size_t spacePos = line.find_first_of(' ');
    if (spacePos == std::string::npos)
    {
        result.errorMessage += std::format("Parse error on file [{}]: Expected space not found on line {}", totalPath, currentLine);
        result.bSuccess = false;
        return;
    }
    std::string type = line.substr(0, spacePos);
    std::string name = StringUtils::Trim(line.substr(spacePos + 1, line.length() - spacePos + 1));

    Graph::EImageFormat format = Graph::CastStringToImageFormat(type);
    Graph::EShaderParameterType eType = format == Graph::EImageFormat::None ? Graph::CastStringToShaderParameterType(type) : Graph::EShaderParameterType::Image;
    if (eType == Graph::EShaderParameterType::None)
    {
        result.errorMessage += std::format("Parse error on file [{}]: Unknown Type: [{}]", totalPath, type);
        result.bSuccess = false;
        return;
    }

    Graph::ShaderParameter shaderParameter;
    shaderParameter.name = name;
    shaderParameter.format = format;
    shaderParameter.type = eType;
    
    result.nodeInfo.uniforms.push_back(shaderParameter);
}

