#include "FGSEngine.h"

#include <cassert>
#include <chrono>
#include <thread>

#include "Assets/TextureLoader.h"
#include "GPU/VkHelpers.h"
#include "GPU/VulkanBuffer.h"
#include "GPU/VulkanDescriptors.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_vulkan.h"
#include "ShaderCompiler/ShaderCompiler.h"
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

    InitSyncStructures();

    InitDefaultSamplers();

    InitTestImage();

    InitDescriptors();

    ShaderCompiler::InitGlslCompiler();

    InitPipelines();
    
    

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
        if (_window.bResizeRequested)
        {
            ResizeSwapchain();
        }
        // TODO: EDITOR UI

        Draw();
    }
}

void FGSEngine::Draw()
{
    // Synchronization
    VK_CHECK(vkWaitForFences(_vulkanContext.device, 1, &GetCurrentFrame().renderFence, true, 1000000000));
    VK_CHECK(vkResetFences(_vulkanContext.device, 1, &GetCurrentFrame().renderFence));

    GetCurrentFrame().deletionStack.Flush();
    GetCurrentFrame().frameDescriptorAllocator.ClearPool(_vulkanContext.device);

    uint32_t swapchainImageIndex;
    VkResult e = vkAcquireNextImageKHR(_vulkanContext.device, _swapchain.swapChain, 1000000000, GetCurrentFrame().acquireSemaphore, nullptr, &swapchainImageIndex);
    if (e == VK_ERROR_OUT_OF_DATE_KHR)
    {
        _window.bResizeRequested = true;
        return;
    }

    // Take and reset the cmd
    VkCommandBuffer cmd = GetCurrentFrame().commandBuffer;
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    _drawExtent.width = std::min(_swapchain.swapchainExtent.width, _drawImage.imageExtent.width);
    _drawExtent.height = std::min(_swapchain.swapchainExtent.height, _drawImage.imageExtent.height);

    // Start the cmd record
    VkCommandBufferBeginInfo cmdBeginInfo = VkHelpers::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    VkHelpers::TransitionImage(cmd, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    // Create the common Buffer and the descriptorSet for it in the Per Frame Resources also fill it
    VulkanBuffer commonValuesBuffer = CreateBuffer(sizeof(float) * 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    GetCurrentFrame().deletionStack.Push([this, commonValuesBuffer]()
    {
        DestroyBuffer(commonValuesBuffer);
    });

    {
        float* commonValues;
        vmaMapMemory(_allocator, commonValuesBuffer.allocation, (void**)&commonValues);
        commonValues[0] = _window.windowExtent.width;
        commonValues[1] = _window.windowExtent.height;
        commonValues[2] = _testImage.imageExtent.width;
        commonValues[3] = _testImage.imageExtent.height;
        vmaUnmapMemory(_allocator, commonValuesBuffer.allocation);
    }

    VkDescriptorSet commonDescriptorSet = GetCurrentFrame().frameDescriptorAllocator.Allocate(_vulkanContext.device, _commonDescriptorSetLayout);
    {
        Descriptors::DescriptorWriter writer;
        writer.WriteBuffer(0, commonValuesBuffer.buffer, sizeof(float) * 4, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

        writer.UpdateSet(_vulkanContext.device, commonDescriptorSet);
    }
    
    // DRAW ITSELF
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _drawPipeline.pipeline);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _drawPipeline.layout, 0, 1, &commonDescriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _drawPipeline.layout, 1, 1, &_drawImageDescriptorSet, 0, nullptr);

    vkCmdDispatch(cmd, std::ceil(_drawExtent.width / 16.0f), std::ceil(_drawExtent.height / 16.0f), 1);
    // END DRAW ITSELF

    VkHelpers::TransitionImage(cmd, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VkHelpers::TransitionImage(cmd, _swapchain.swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkHelpers::CopyImageToImage(cmd, _drawImage.image, _swapchain.swapchainImages[swapchainImageIndex], _drawExtent, _swapchain.swapchainExtent);

    VkHelpers::TransitionImage(cmd, _swapchain.swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VK_CHECK(vkEndCommandBuffer(cmd));

    // PREPARE THE SUBMISION
    VkCommandBufferSubmitInfo cmdSubmitInfo = VkHelpers::CommandBufferSubmitInfo(cmd);

    VkSemaphoreSubmitInfo waitInfo = VkHelpers::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, GetCurrentFrame().acquireSemaphore);
    VkSemaphoreSubmitInfo signalInfo = VkHelpers::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, _swapchain.swapchainSubmitSemaphores[swapchainImageIndex]);

    VkSubmitInfo2 submit = VkHelpers::SubmitInfo(&cmdSubmitInfo, &signalInfo, &waitInfo);

    VK_CHECK(vkQueueSubmit2(_mainQueue.queue, 1, &submit, GetCurrentFrame().renderFence));

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;

    presentInfo.pSwapchains = &_swapchain.swapChain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &_swapchain.swapchainSubmitSemaphores[swapchainImageIndex];
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapchainImageIndex;

    VkResult presentResult = vkQueuePresentKHR(_mainQueue.queue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        _window.bResizeRequested = true;
    }
    
    _frameNumber++;
}

void FGSEngine::CleanUp()
{
    if (!_bIsInitialized) return;

    ShaderCompiler::ShutdownGlslCompiler();
    
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
    // Swapchain commands
    {
       VkCommandPoolCreateInfo commandPoolInfo =  VkHelpers::CommandPoolCreateInfo(_mainQueue.familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        for (int i = 0; i < FRAME_OVERLAP; ++i)
        {
            VK_CHECK(vkCreateCommandPool(_vulkanContext.device, &commandPoolInfo, nullptr, &_frames[i].commandPool));

            VkCommandBufferAllocateInfo cmdAllocInfo = VkHelpers::CommandBufferAllocateInfo(_frames[i].commandPool, 1);

            VK_CHECK(vkAllocateCommandBuffers(_vulkanContext.device, &cmdAllocInfo, &_frames[i].commandBuffer));
        }
    }

    // Immediate submits commands
    {
        VkCommandPoolCreateInfo commandPoolInfo =  VkHelpers::CommandPoolCreateInfo(_immediateQueue.familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        VK_CHECK(vkCreateCommandPool(_vulkanContext.device, &commandPoolInfo, nullptr, &_immediateCommandPool));

        VkCommandBufferAllocateInfo cmdAllocInfo = VkHelpers::CommandBufferAllocateInfo(_immediateCommandPool, 1);

        VK_CHECK(vkAllocateCommandBuffers(_vulkanContext.device, &cmdAllocInfo, &_immediateCommandBuffer));

        _mainDeletionStack.Push([this]()
        {
            vkDestroyCommandPool(_vulkanContext.device, _immediateCommandPool, nullptr);
        });
    }
    
}

void FGSEngine::InitSyncStructures()
{
    // Swapchain
    VkFenceCreateInfo fenceCreateInfo = VkHelpers::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = VkHelpers::SemaphoreCreateInfo();

    for (int i = 0; i < FRAME_OVERLAP; ++i)
    {
        VK_CHECK(vkCreateFence(_vulkanContext.device, &fenceCreateInfo, nullptr, &_frames[i].renderFence));

        VK_CHECK(vkCreateSemaphore(_vulkanContext.device, &semaphoreCreateInfo, nullptr, &_frames[i].acquireSemaphore));
    }

    _swapchain.swapchainSubmitSemaphores.clear();
    _swapchain.swapchainSubmitSemaphores.reserve(_swapchain.swapchainImages.size());
    for (int i = 0; i < _swapchain.swapchainImages.size(); ++i)
    {
        _swapchain.swapchainSubmitSemaphores.push_back(VkSemaphore());
        VK_CHECK(vkCreateSemaphore(_vulkanContext.device, &semaphoreCreateInfo, nullptr, &_swapchain.swapchainSubmitSemaphores[i]));
    }

    // Immediate submit
    VK_CHECK(vkCreateFence(_vulkanContext.device, &fenceCreateInfo, nullptr, &_immediateFence));
    _mainDeletionStack.Push([this]()
    {
        vkDestroyFence(_vulkanContext.device, _immediateFence, nullptr);
    });
}

void FGSEngine::InitDescriptors()
{
    std::vector<Descriptors::PoolSizeRatio> sizes =
        {
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 }
        };

    _drawImageAllocator.InitPool(_vulkanContext.device, 10, sizes);

    // Init the Draw DescriptorSet
    {
        Descriptors::DescriptorLayoutBuilder builder;
        builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        builder.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        _drawImageDescriptorSetLayout = builder.Build(_vulkanContext.device);
    }

    _drawImageDescriptorSet = _drawImageAllocator.Allocate(_vulkanContext.device, _drawImageDescriptorSetLayout);

    // Write the draw image into the set
    {
        Descriptors::DescriptorWriter writer;
        writer.WriteImage(0, _drawImage.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        writer.WriteImage(1, _testImage.imageView, _defaultSamplerLinear, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        
        writer.UpdateSet(_vulkanContext.device, _drawImageDescriptorSet);
    }

    _mainDeletionStack.Push([this]()
    {
        _drawImageAllocator.DestroyPool(_vulkanContext.device); // destroy the pool so all his descriptor sets are also destroyed

        vkDestroyDescriptorSetLayout(_vulkanContext.device, _drawImageDescriptorSetLayout, nullptr);
    });

    // Init the DescriptorSet
    for (unsigned int i = 0; i < FRAME_OVERLAP; ++i)
    {
        std::vector<Descriptors::PoolSizeRatio> framePoolSizes =
            {
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 }
            };
        
        _frames[i].frameDescriptorAllocator.InitPool(_vulkanContext.device, 1, framePoolSizes);

        _mainDeletionStack.Push([this, i]()
        {
            _frames[i].frameDescriptorAllocator.DestroyPool(_vulkanContext.device);
        });
    }

    // Descriptor set with that buffer
    {
        Descriptors::DescriptorLayoutBuilder builder;
        builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        _commonDescriptorSetLayout = builder.Build(_vulkanContext.device);
    }

    _mainDeletionStack.Push([this]()
    {
        vkDestroyDescriptorSetLayout(_vulkanContext.device, _commonDescriptorSetLayout, nullptr);
    });
}

void FGSEngine::InitPipelines()
{
    // TODO: Test for see the shaders compilation

    ShaderCompiler::ShaderCompilationResult compilationResult = ShaderCompiler::CompileGlslFileIntoSpirV("sampleImage.comp");

    if (!compilationResult.bSuccess)
    {
        Logger::Log(Logger::LogLevel::Error, "Failed to compile sampleImage.comp, Not continue with pipeline creation");
        return;
    }

    VkDescriptorSetLayout descriptorSetLayouts[] = { _commonDescriptorSetLayout, _drawImageDescriptorSetLayout };
    
    VkPipelineLayoutCreateInfo computeLayoutCreateInfo {};
    computeLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computeLayoutCreateInfo.pNext = nullptr;
    computeLayoutCreateInfo.pSetLayouts = descriptorSetLayouts;
    computeLayoutCreateInfo.setLayoutCount = 2;

    VkPipelineLayout computeLayoutTemp;
    VK_CHECK(vkCreatePipelineLayout(_vulkanContext.device, &computeLayoutCreateInfo, nullptr, &computeLayoutTemp));

    VkShaderModule gradientShaderModule;
    if (!VkHelpers::LoadShaderModule(_vulkanContext.device, compilationResult.spirV, &gradientShaderModule))
    {
        Logger::Log(Logger::LogLevel::Warning, "Failed to load sampleImage shader module");
    }

    VkPipelineShaderStageCreateInfo stageInfo {};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.pNext = nullptr;
    stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageInfo.module = gradientShaderModule;
    stageInfo.pName = "main"; // Entry point of the shader

    VkComputePipelineCreateInfo pipelineCreateInfo {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = nullptr;
    pipelineCreateInfo.layout = computeLayoutTemp;
    pipelineCreateInfo.stage = stageInfo;

    _drawPipeline.layout = computeLayoutTemp;
    
    VK_CHECK(vkCreateComputePipelines(_vulkanContext.device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &_drawPipeline.pipeline));

    _mainDeletionStack.Push([this, computeLayoutTemp]()
    {
        vkDestroyPipelineLayout(_vulkanContext.device, computeLayoutTemp, nullptr);
        vkDestroyPipeline(_vulkanContext.device, _drawPipeline.pipeline, nullptr);
    });
}

void FGSEngine::InitDefaultSamplers()
{
    VkSamplerCreateInfo samplerInfo = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;

    VK_CHECK(vkCreateSampler(_vulkanContext.device, &samplerInfo, nullptr, &_defaultSamplerNearest));

    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    VK_CHECK(vkCreateSampler(_vulkanContext.device, &samplerInfo, nullptr, &_defaultSamplerLinear));

    _mainDeletionStack.Push([this]()
    {
        vkDestroySampler(_vulkanContext.device, _defaultSamplerNearest, nullptr);
        vkDestroySampler(_vulkanContext.device, _defaultSamplerLinear, nullptr);
    });
}

void FGSEngine::InitTestImage()
{
    TextureLoader::TextureResult result;
    
    if (!TextureLoader::ReadTextureFromFile("defaultTexture.png", &result))
    {
        Logger::Log(Logger::LogLevel::Error, "Failed to load texture from file: defaultTexture.png");
        TextureLoader::FreeTextureResult(&result);
        return;
    }

    _testImage = CreateAndFillImage(&result, VK_IMAGE_USAGE_SAMPLED_BIT);
    TextureLoader::FreeTextureResult(&result);
}

void FGSEngine::InitUI()
{
    
}

void FGSEngine::DestroySwapchain()
{
    // This call also destroy the swapchain images because they are internal to it
    vkDestroySwapchainKHR(_vulkanContext.device, _swapchain.swapChain, nullptr);

    for (size_t i = 0; i < _swapchain.swapchainImageViews.size(); ++i)
    {
        vkDestroyImageView(_vulkanContext.device, _swapchain.swapchainImageViews[i], nullptr);
    }
}

void FGSEngine::ResizeSwapchain()
{
    vkDeviceWaitIdle(_vulkanContext.device);

    DestroySwapchain();

    int w, h;
    SDL_GetWindowSize(_window.window, &w, &h);
    _window.windowExtent.width = w;
    _window.windowExtent.height = h;

    _swapchain.Create(_vulkanContext, _window.windowExtent.width, _window.windowExtent.height);

    _window.bResizeRequested = false;
}

void FGSEngine::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
{
    VK_CHECK(vkResetFences(_vulkanContext.device, 1, &_immediateFence));
    VK_CHECK(vkResetCommandBuffer(_immediateCommandBuffer, 0));

    VkCommandBuffer cmd = _immediateCommandBuffer;

    VkCommandBufferBeginInfo cmdBeginInfo = VkHelpers::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    function(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmdSubmitInfo = VkHelpers::CommandBufferSubmitInfo(cmd);
    VkSubmitInfo2 submit = VkHelpers::SubmitInfo(&cmdSubmitInfo, nullptr, nullptr);

    VK_CHECK(vkQueueSubmit2(_immediateQueue.queue, 1, &submit, _immediateFence));

    VK_CHECK(vkWaitForFences(_vulkanContext.device, 1, &_immediateFence, true, 9999999999));
}

VulkanImage FGSEngine::CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usageFlags)
{
    VulkanImage newImage;
    newImage.imageFormat = format;
    newImage.imageExtent = size;

    VkImageCreateInfo imageInfo = VkHelpers::ImageCreateInfo(newImage.imageFormat, usageFlags, newImage.imageExtent);
    
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vmaCreateImage(_allocator, &imageInfo, &allocInfo, &newImage.image, &newImage.allocation, nullptr));

    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageViewCreateInfo viewInfo = VkHelpers::ImageViewCreateInfo(newImage.imageFormat, newImage.image, aspectFlags);

    VK_CHECK(vkCreateImageView(_vulkanContext.device, &viewInfo, nullptr, &newImage.imageView));

    return newImage;
}

VulkanImage FGSEngine::CreateAndFillImage(TextureLoader::TextureResult* textureData, VkImageUsageFlags usageFlags)
{
    size_t dataSize = textureData->width * textureData->height * 4;
    VulkanBuffer uploadBuffer = CreateBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    std::memcpy(uploadBuffer.allocationInfo.pMappedData, textureData->data, dataSize);

    VkExtent3D extent;
    extent.width = textureData->width;
    extent.height = textureData->height;
    extent.depth = 1;
    
    VulkanImage newImage = CreateImage(extent, VK_FORMAT_R8G8B8A8_UNORM, usageFlags | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    
    ImmediateSubmit([&](VkCommandBuffer cmd)
    {
        VkHelpers::TransitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkBufferImageCopy copyRegion = {};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;

        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageExtent = newImage.imageExtent;

        vkCmdCopyBufferToImage(cmd, uploadBuffer.buffer, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
        
        VkHelpers::TransitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });

    DestroyBuffer(uploadBuffer);

    return newImage;
}

VulkanBuffer FGSEngine::CreateBuffer(size_t allocSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage)
{
    VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .pNext = nullptr };
    bufferInfo.size = allocSize;
    bufferInfo.usage = usageFlags;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsage;
    allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VulkanBuffer newBuffer;

    VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &allocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.allocationInfo));

    return newBuffer;
}

void FGSEngine::DestroyBuffer(const VulkanBuffer& buffer)
{
    vmaDestroyBuffer(_allocator, buffer.buffer, buffer.allocation);
}
