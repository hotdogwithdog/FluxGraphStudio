#pragma once

#include "VkTypes.h"

struct VulkanBuffer
{
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
};