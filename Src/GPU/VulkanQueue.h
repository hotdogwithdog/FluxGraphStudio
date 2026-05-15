#pragma once

#include "VkTypes.h"

struct VulkanQueue
{
    VkQueue queue {VK_NULL_HANDLE};
    uint32_t familyIndex = 0;
};
