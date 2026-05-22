#include "VkHelpers.h"

VkImageCreateInfo VkHelpers::ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
{
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;

    info.imageType = VK_IMAGE_TYPE_2D;

    info.format = format;
    info.extent = extent;

    info.mipLevels = 1;
    info.arrayLayers = 1;

    info.samples = VK_SAMPLE_COUNT_1_BIT;

    info.tiling = VK_IMAGE_TILING_OPTIMAL; // TODO: The tiling for CPU Readbacks must be Linear for store it in an array format maybe added later on
    info.usage = usageFlags;

    return info;
}

VkImageViewCreateInfo VkHelpers::ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;

    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.image = image;
    info.format = format;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.aspectMask = aspectFlags;

    return info;
}

VkCommandPoolCreateInfo VkHelpers::CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
    VkCommandPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.queueFamilyIndex = queueFamilyIndex;
    info.flags = flags;
    return info;
}

VkCommandBufferAllocateInfo VkHelpers::CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count)
{
    VkCommandBufferAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.commandPool = pool;
    info.commandBufferCount = count;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    return info;
}

VkFenceCreateInfo VkHelpers::FenceCreateInfo(VkFenceCreateFlags flags)
{
    VkFenceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    return info;
}

VkSemaphoreCreateInfo VkHelpers::SemaphoreCreateInfo(VkSemaphoreCreateFlags flags)
{
    VkSemaphoreCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    return info;
}

VkSemaphoreSubmitInfo VkHelpers::SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
{
    VkSemaphoreSubmitInfo info {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    info.pNext = nullptr;
    info.semaphore = semaphore;
    info.stageMask = stageMask;
    info.deviceIndex = 0;
    info.value = 1;
    
    return info;
}

VkSubmitInfo2 VkHelpers::SubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo)
{
    VkSubmitInfo2 info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    info.pNext = nullptr;

    info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
    info.pWaitSemaphoreInfos = waitSemaphoreInfo;

    info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
    info.pSignalSemaphoreInfos = signalSemaphoreInfo;

    info.commandBufferInfoCount = 1;
    info.pCommandBufferInfos = cmd;

    return info;
}

bool VkHelpers::LoadShaderModule(VkDevice device, const std::vector<uint32_t>& spirv, VkShaderModule* outShaderModule)
{
    if (spirv.size() == 0) return false;

    VkShaderModuleCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.pNext = nullptr;

    info.codeSize = spirv.size() * sizeof(uint32_t);
    info.pCode = spirv.data();

    if (vkCreateShaderModule(device, &info, nullptr, outShaderModule) != VK_SUCCESS)
    {
        return false;
    }
    
    return true;
}

VkCommandBufferBeginInfo VkHelpers::CommandBufferBeginInfo(VkCommandBufferUsageFlags flags /*= 0*/)
{
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;

    info.pInheritanceInfo = nullptr;
    info.flags = flags;
    return info;
}

VkCommandBufferSubmitInfo VkHelpers::CommandBufferSubmitInfo(VkCommandBuffer cmd)
{
    VkCommandBufferSubmitInfo info {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    info.pNext = nullptr;
    info.commandBuffer = cmd;
    info.deviceMask = 0;
    
    return info;
}

void VkHelpers::TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier2 imageBarrier = { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, };
    imageBarrier.pNext = nullptr;

    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; // TODO: Optimize this transitions
    imageBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

    imageBarrier.oldLayout = currentLayout;
    imageBarrier.newLayout = newLayout;

    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // TODO: if for some reason want to support depth change this
    imageBarrier.subresourceRange = ImageSubresourceRange(aspectMask);
    imageBarrier.image = image;

    VkDependencyInfo dependencyInfo {};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.pNext = nullptr;

    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &imageBarrier;
    
    vkCmdPipelineBarrier2(cmd, &dependencyInfo);
}

VkImageSubresourceRange VkHelpers::ImageSubresourceRange(VkImageAspectFlags aspectMask)
{
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = aspectMask;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    return subresourceRange;
}

void VkHelpers::CopyImageToImage(VkCommandBuffer cmd, VkImage sourceImage, VkImage destImage, VkExtent2D sourceSize, VkExtent2D destSize)
{
    VkImageBlit2 blitRegion { .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

    blitRegion.srcOffsets[1].x = sourceSize.width;
    blitRegion.srcOffsets[1].y = sourceSize.height;
    blitRegion.srcOffsets[1].z = 1;

    blitRegion.dstOffsets[1].x = destSize.width;
    blitRegion.dstOffsets[1].y = destSize.height;
    blitRegion.dstOffsets[1].z = 1;

    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcSubresource.mipLevel = 0;

    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstSubresource.mipLevel = 0;

    VkBlitImageInfo2 blitInfo { .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };

    blitInfo.srcImage = sourceImage;
    blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    blitInfo.dstImage = destImage;
    blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    blitInfo.filter = VK_FILTER_LINEAR;
    blitInfo.regionCount = 1;
    blitInfo.pRegions = &blitRegion;

    vkCmdBlitImage2(cmd, &blitInfo);
}

VkRenderingAttachmentInfo VkHelpers::AttachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout /*= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL*/)
{
    VkRenderingAttachmentInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    info.pNext = nullptr;

    info.imageView = view;
    info.imageLayout = layout;
    info.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    if (clear) info.clearValue = *clear;

    return info;
}

VkRenderingInfo VkHelpers::RenderingInfo(VkExtent2D extent, VkRenderingAttachmentInfo* colorAttachment)
{
    VkRenderingInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    info.pNext = nullptr;

    info.renderArea = VkRect2D { VkOffset2D {0, 0}, extent };
    info.layerCount = 1;
    info.colorAttachmentCount = 1;
    info.pColorAttachments = colorAttachment;
    info.pDepthAttachment = nullptr;
    info.pStencilAttachment = nullptr;

    return info;
}
