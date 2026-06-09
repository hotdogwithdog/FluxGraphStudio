#include "FormatUtils.h"


VkFormat FormatUtils::CastEImageFormatToVkFormat(Graph::EImageFormat format)
{
    switch (format)
    {
        case Graph::EImageFormat::RGBA16F:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case Graph::EImageFormat::RGBA32F:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Graph::EImageFormat::RGBA8:
                return VK_FORMAT_R8G8B8A8_SRGB;
        case Graph::EImageFormat::R8:
            return VK_FORMAT_R8_UINT;
        case Graph::EImageFormat::R16F:
            return VK_FORMAT_R16_SFLOAT;
        case Graph::EImageFormat::RG16F:
            return VK_FORMAT_R16G16_SFLOAT;
        default:
            return VK_FORMAT_UNDEFINED;
    }
}

Graph::EImageFormat FormatUtils::CastVkFormatToEImageFormat(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return Graph::EImageFormat::RGBA16F;
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return Graph::EImageFormat::RGBA32F;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return Graph::EImageFormat::RGBA8;
        case VK_FORMAT_R8_UINT:
            return Graph::EImageFormat::R8;
        case VK_FORMAT_R16_SFLOAT:
            return Graph::EImageFormat::R16F;
        case VK_FORMAT_R16G16_SFLOAT:
            return Graph::EImageFormat::RG16F;
        default:
            return Graph::EImageFormat::None;
    }
}
