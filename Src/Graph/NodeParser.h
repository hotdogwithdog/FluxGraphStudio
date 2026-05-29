#pragma once

#include "RGNodeInfo.h"
#include "ShaderCompiler/ShaderCompiler.h"


namespace NodeParser
{
    
    struct NodeParserResult
    {
        bool bSuccess;
        std::string errorMessage;
        RGNodeInfo nodeInfo;
        ShaderCompiler::ShaderCompilationResult compilationResult;
    };

    NodeParserResult CompileGNodeFile(std::filesystem::path path);

    
}
