#pragma once
#include <vulkan/vulkan_core.h>

#include "Graph/RGNodeInfo.h"

namespace FormatUtils
{
    VkFormat CastEImageFormatToVkFormat(Graph::EImageFormat format);
    Graph::EImageFormat CastVkFormatToEImageFormat(VkFormat format);
}
