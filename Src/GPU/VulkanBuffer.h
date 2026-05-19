#pragma once

#include "VkTypes.h"
#include "vma/vk_mem_alloc.h"

struct VulkanBuffer
{
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
};