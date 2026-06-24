#pragma once

#include "glslang/Public/ShaderLang.h"

class FGSIncluder : public glslang::TShader::Includer
{
public:

    IncludeResult* includeSystem(const char* headerName, const char* includeName, size_t inclusionDepth) override;

    IncludeResult* includeLocal(const char*, const char*, size_t) override;
    
    void releaseInclude(IncludeResult* result) override;
};
