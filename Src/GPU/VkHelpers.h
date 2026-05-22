#pragma once

#include "VkTypes.h"
#include <vector>

namespace VkHelpers
{
    VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
    VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

    VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
    VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1);

    VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = 0);
    VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

    VkSemaphoreSubmitInfo SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);

    VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);

    bool LoadShaderModule(VkDevice device, const std::vector<uint32_t>& spirv, VkShaderModule* outShaderModule);

    VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);

    VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd);

    void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
    
    VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags aspectMask);

    void CopyImageToImage(VkCommandBuffer cmd, VkImage sourceImage, VkImage destImage, VkExtent2D sourceSize, VkExtent2D destSize);

    VkRenderingAttachmentInfo AttachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo RenderingInfo(VkExtent2D extent, VkRenderingAttachmentInfo* colorAttachment);
    
}
