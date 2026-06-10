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
    };

    NodeParserResult ParseGNodeFile(std::filesystem::path path);

    
}
