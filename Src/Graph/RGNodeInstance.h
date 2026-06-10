#pragma once

#include "GPU/GPUResourceManager.h"

struct RGNodeInstance
{
    RGNodeHandle nodeHandle;
    
    std::vector<GPUImageHandle> uniformImages;
    std::vector<GPUBufferHandle> uniformBuffers;

    VkDescriptorSet descriptorSet;
    Descriptors::DescriptorAllocator descriptorAllocator;
};
