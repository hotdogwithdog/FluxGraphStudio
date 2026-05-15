#pragma once

#include <vector>

#include "VkTypes.h"

struct VulkanContext;

class VulkanSwapchain
{
public:
    VkSwapchainKHR swapChain;
    VkFormat imageFormat;

    std::vector<VkSemaphore> swapchainSubmitSemaphores;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkExtent2D swapchainExtent;

public:
    void Create(const VulkanContext& context, uint32_t width, uint32_t height);


};
