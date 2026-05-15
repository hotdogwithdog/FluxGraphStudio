#include "VulkanSwapchain.h"

#include "VkBootstrap.h"
#include "VulkanContext.h"

void VulkanSwapchain::Create(const VulkanContext& context, uint32_t width, uint32_t height)
{
    vkb::SwapchainBuilder swapChainbuilder { context.physicalDevice, context.device, context.surface };

    imageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapChainbuilder
        .set_desired_format(VkSurfaceFormatKHR { .format = imageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(width, height)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

    swapchainExtent = vkbSwapchain.extent;
    swapChain = vkbSwapchain.swapchain;
    swapchainImages = vkbSwapchain.get_images().value();
    swapchainImageViews = vkbSwapchain.get_image_views().value();
}
