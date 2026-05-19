#pragma once

#include "VkTypes.h"
#include "VulkanDescriptors.h"
#include "Core/DeletionStack.h"

struct FrameData
{
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VkSemaphore acquireSemaphore; // Semaphore that is used for wait the acquireNextImageKHR
    VkFence renderFence; // Fence that is used for wait to this frame to be usable

    DeletionStack deletionStack; // Per frame deletion stack for per frame resources
    Descriptors::DescriptorAllocator frameDescriptorAllocator; // Basically used only for Common Uniform buffer of data that all shaders shared (maybe change it to push constants it fits well)
};
