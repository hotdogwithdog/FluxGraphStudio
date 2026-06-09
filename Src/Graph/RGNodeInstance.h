#pragma once

#include "Assets/AssetsManager.h"
#include "GPU/GPUResourceManager.h"

struct RGNodeInstance
{
    NodeInfoHandle _nodeInfoHandle;
    
    std::vector<GPUImageHandle> _uniformImages;
    std::vector<GPUBufferHandle> _uniformBuffers;

    Descriptors::DescriptorAllocator _descriptorAllocator;
};
