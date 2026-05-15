#include "FGSEngine.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <ostream>
#include <thread>

#include "GPU/VkHelpers.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_vulkan.h"
#include "VkBootstrap.h"


FGSEngine* loadedEngine = nullptr;
FGSEngine* FGSEngine::GetInstance() { return loadedEngine; }


void FGSEngine::Init()
{
    assert(loadedEngine == nullptr);
    loadedEngine = this;

    SDL_Init(SDL_INIT_VIDEO);

    _window.Init();

    InitVulkan();

    InitSwapChain();

    InitCommands();
    

    _bIsInitialized = true;
}

void FGSEngine::Run()
{
    SDL_Event e;
    bool bQuit = false;
    
    while (!bQuit)
    {

        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_EVENT_QUIT)
            {
                bQuit = true;
            }

            if (e.type == SDL_EVENT_WINDOW_MINIMIZED)
            {
                _window.isMinimized = true;
            }
            if (e.type == SDL_EVENT_WINDOW_RESTORED)
            {
                _window.isMinimized = false;
            }
        }

        if (_window.isMinimized)
        {
            // Sleep for a little to not overload the CPU while the app si minimized // TODO: Probably can be done in a really better way
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // TODO: Resize

        // TODO: EDITOR UI

        Draw();
    }
}

void FGSEngine::Draw()
{
    Logger::Log(Logger::LogLevel::Debug, "Drawing");
}

void FGSEngine::CleanUp()
{
    if (!_bIsInitialized) return;

    
}

void FGSEngine::InitVulkan()
{
    vkb::InstanceBuilder builder;

    auto inst_ret = builder.set_app_name("FluxGraphStudio")
        .request_validation_layers(_bValidationLayers)
        .use_default_debug_messenger()
        .require_api_version(1, 3, 0)
        .build();

    vkb::Instance vkb_inst = inst_ret.value();

    _vulkanContext.instance = vkb_inst.instance;
    _vulkanContext.debugMessenger = vkb_inst.debug_messenger;

    SDL_Vulkan_CreateSurface(_window.window, _vulkanContext.instance, nullptr, &_vulkanContext.surface);

    // Vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features13 { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    features13.dynamicRendering = true;
    features13.synchronization2 = true;

    // Vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12 { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    vkb::PhysicalDeviceSelector selector { vkb_inst };
    vkb::PhysicalDevice physicalDevice = selector
        .set_minimum_version(1, 3)
        .set_required_features_13(features13)
        .set_required_features_12(features12)
        .set_surface(_vulkanContext.surface)
        .select()
        .value();

    vkb::DeviceBuilder deviceBuilder { physicalDevice };
    vkb::Device vkbDevice = deviceBuilder.build().value();

    _vulkanContext.device = vkbDevice.device;
    _vulkanContext.physicalDevice = physicalDevice.physical_device;

    _mainQueue.queue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _mainQueue.familyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    _immediateQueue.queue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _immediateQueue.familyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    // Init of VMA
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _vulkanContext.physicalDevice;
    allocatorInfo.device = _vulkanContext.device;
    allocatorInfo.instance = _vulkanContext.instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &_allocator);
    
    _mainDeletionStack.Push([this]()
    {
       vmaDestroyAllocator(_allocator); 
    });
}

void FGSEngine::InitSwapChain()
{
    _swapchain.Create(_vulkanContext, _window.windowExtent.width, _window.windowExtent.height);

    // Create the DrawImage
    // TODO: Maybe move this to his own function because is not the swapchain even if is related for be the image of draw that later on is copied to the swapchain image
    VkExtent3D drawImageExtent = { _window.windowExtent.width, _window.windowExtent.height, 1 };

    _drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    _drawImage.imageExtent = drawImageExtent;

    VkImageUsageFlags drawImageUsageFlags{};
    drawImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo drawImageInfo = VkHelpers::ImageCreateInfo(_drawImage.imageFormat, drawImageUsageFlags, _drawImage.imageExtent);

    VmaAllocationCreateInfo drawImageAllocInfo = {};
    drawImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    drawImageAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    vmaCreateImage(_allocator, &drawImageInfo, &drawImageAllocInfo, &_drawImage.image, &_drawImage.allocation, nullptr);

    VkImageViewCreateInfo drawImageViewCreateInfo = VkHelpers::ImageViewCreateInfo(_drawImage.imageFormat, _drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

    VK_CHECK(vkCreateImageView(_vulkanContext.device, &drawImageViewCreateInfo, nullptr, &_drawImage.imageView));
    
    _mainDeletionStack.Push([this]()
    {
        vkDestroyImageView(_vulkanContext.device, _drawImage.imageView, nullptr);
        vmaDestroyImage(_allocator, _drawImage.image, _drawImage.allocation);
    });
}

void FGSEngine::InitCommands()
{
    
}

void FGSEngine::InitUI()
{
    
}

void FGSEngine::CreateSwapChain(uint32_t width, uint32_t height)
{
    
}
