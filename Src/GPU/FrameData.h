#pragma once

#include "VkTypes.h"
#include "Core/DeletionStack.h"

struct FrameData
{
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VkSemaphore acquireSemaphore; // Semaphore that is used for wait the acquireNextImageKHR
    VkFence renderFence; // Fence that is used for wait to this frame to be usable

    DeletionStack deletionStack; // Per frame deletion stack for per frame resources
    //DescriptorAllocatorGrowable frameDescriptorAllocator; // TODO: Des comment this line
};
