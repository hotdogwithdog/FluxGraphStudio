#pragma once

#include <string>
#include <vector>

namespace ShaderCompiler
{
    enum CompilationFlags
    {
        None = 0,
        OptimizePerformance = 1,
        OptimizeSize = (1 << 1),
        NoOptimize = (1 << 2),
    };

    
    std::vector<uint32_t> CompileGlslFile(const std::string&, CompilationFlags flags = CompilationFlags::NoOptimize);
}
