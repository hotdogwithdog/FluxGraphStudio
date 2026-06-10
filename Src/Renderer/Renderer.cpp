#include "Renderer.h"

#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>

#include "Assets/TextureLoader.h"
#include "Core/Window.h"
#include "SDL3/SDL_vulkan.h"
#include "ShaderCompiler/ShaderCompiler.h"
#include "VkBootstrap.h"
#include "Assets/AssetsManager.h"
#include "Editor/Editor.h"
#include "Graph/DescriptionGraph.h"
#include "Graph/DescriptionNodeInstance.h"
#include "Graph/NodeParser.h"
#include "SDL3/SDL_init.h"

void Renderer::Init(Window* window)
{
    assert(_bIsInitialized == false);
    
    _window = window;

    SDL_Init(SDL_INIT_VIDEO);

    _window->Init();

    InitVulkan();

    InitSwapChain();

    InitPreviewImage();

    InitCommands();

    InitSyncStructures();

    InitDefaultSamplers();

    InitSourceImage();

    // TODO: remove this is just for testing
    NodeParser::NodeParserResult result = NodeParser::CompileGNodeFile("test.gNode");
    NodeInfoHandle handle;
    if (!result.bSuccess)
    {
        Logger::Log(Logger::LogLevel::Error, result.errorMessage);
    }
    else
    {
        handle = AssetsManager::RegisterNodeInfo(result.nodeInfo);
    }

    DescriptionGraph graph;

    DescriptionNodeInstance nodeInstanceA;
    nodeInstanceA.nodeInfoHandle = handle;
    DescriptionNodeInstance nodeInstanceB;
    nodeInstanceB.nodeInfoHandle = handle;

    DescriptionNodeInstanceHandle nodeA = graph.AddNodeInstance(nodeInstanceA);
    DescriptionNodeInstanceHandle nodeB = graph.AddNodeInstance(nodeInstanceB);

    graph.ConnectNodes(nodeA, 0, nodeB, 0);
    
    graph.MarkNodeAsStart(nodeA, 0);

    auto compiledGraph = graph.CompileGraph();
    
    
    InitDescriptors();

    ShaderCompiler::InitGlslCompiler();

    InitPipelines();

    InitImGui();
    
    _sourceImage.ID = (ImTextureID)ImGui_ImplVulkan_AddTexture(_defaultSamplerLinear, _sourceImage.resource.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    _previewImage.ID = (ImTextureID)ImGui_ImplVulkan_AddTexture(_defaultSamplerLinear, _previewImage.resource.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    _bIsInitialized = true;
}

void Renderer::Render()
{
    if (_window->bResizeRequested)
    {
        ResizeSwapchain();
    }
    
    Draw();
}

void Renderer::CleanUp()
{
    if (!_bIsInitialized) return;

    ShaderCompiler::ShutdownGlslCompiler();

    
}



void Renderer::InitVulkan()
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

    SDL_Vulkan_CreateSurface(_window->window, _vulkanContext.instance, nullptr, &_vulkanContext.surface);

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

void Renderer::InitSwapChain()
{
    _swapchain.Create(_vulkanContext, _window->windowExtent.width, _window->windowExtent.height);
}

void Renderer::InitPreviewImage()
{
    // Create the PreviewImage
    VkExtent3D previewImageExtent = { 1920, 1080, 1 };

    VkImageUsageFlags previewImageUsageFlags{};
    previewImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    previewImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    previewImageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
    previewImageUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    previewImageUsageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    
    _previewImage.resource = CreateImage(previewImageExtent, VK_FORMAT_R16G16B16A16_SFLOAT, previewImageUsageFlags);
    
    _mainDeletionStack.Push([this]()
    {
        vkDestroyImageView(_vulkanContext.device, _previewImage.resource.imageView, nullptr);
        vmaDestroyImage(_allocator, _previewImage.resource.image, _previewImage.resource.allocation);
    });
}

void Renderer::InitCommands()
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

void Renderer::InitSyncStructures()
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

void Renderer::InitDescriptors()
{
    // TODO: This will change to the graph system and will not have a default descriptor allocator instantiated it will be per node, allocator and descriptor set
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

    // Write the preview Image into the set
    {
        Descriptors::DescriptorWriter writer;
        writer.WriteImage(0, _previewImage.resource.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        writer.WriteImage(1, _sourceImage.resource.imageView, _defaultSamplerLinear, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        
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

void Renderer::InitPipelines()
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

void Renderer::InitDefaultSamplers()
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

void Renderer::InitSourceImage()
{
    TextureLoader::TextureResult result;
    
    if (!TextureLoader::ReadTextureFromFile("defaultTexture.png", &result))
    {
        Logger::Log(Logger::LogLevel::Error, "Failed to load texture from file: defaultTexture.png");
        TextureLoader::FreeTextureResult(&result);
        return;
    }

    _sourceImage.resource = CreateAndFillImage(&result, VK_IMAGE_USAGE_SAMPLED_BIT);
    TextureLoader::FreeTextureResult(&result);

    _mainDeletionStack.Push([this]()
    {
        vkDestroyImageView(_vulkanContext.device, _sourceImage.resource.imageView, nullptr);
        vkDestroyImage(_vulkanContext.device, _sourceImage.resource.image, nullptr);
    });
}

void Renderer::InitImGui()
{
    VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, .pNext = nullptr };
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
    poolInfo.pPoolSizes = poolSizes;

    VkDescriptorPool imguiPool;
    VK_CHECK(vkCreateDescriptorPool(_vulkanContext.device, &poolInfo, nullptr, &imguiPool));

    ImGui::CreateContext();

    // Activate docking
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; 
    
    ImGui_ImplSDL3_InitForVulkan(_window->window);

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = _vulkanContext.instance;
    initInfo.PhysicalDevice = _vulkanContext.physicalDevice;
    initInfo.Device = _vulkanContext.device;
    initInfo.Queue = _mainQueue.queue;
    initInfo.DescriptorPool = imguiPool;
    initInfo.MinImageCount = 3;
    initInfo.ImageCount = 3;
    initInfo.UseDynamicRendering = true;

    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR, .pNext = nullptr };
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &_swapchain.imageFormat;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);

    _mainDeletionStack.Push([this, imguiPool]()
    {
        ImGui_ImplVulkan_Shutdown();
        vkDestroyDescriptorPool(_vulkanContext.device, imguiPool, nullptr);
    });
}


void Renderer::DestroySwapchain()
{
    // This call also destroy the swapchain images because they are internal to it
    vkDestroySwapchainKHR(_vulkanContext.device, _swapchain.swapChain, nullptr);

    for (size_t i = 0; i < _swapchain.swapchainImageViews.size(); ++i)
    {
        vkDestroyImageView(_vulkanContext.device, _swapchain.swapchainImageViews[i], nullptr);
    }
}

void Renderer::ResizeSwapchain()
{
    vkDeviceWaitIdle(_vulkanContext.device);

    DestroySwapchain();

    int w, h;
    SDL_GetWindowSize(_window->window, &w, &h);
    _window->windowExtent.width = w;
    _window->windowExtent.height = h;

    _swapchain.Create(_vulkanContext, _window->windowExtent.width, _window->windowExtent.height);

    _window->bResizeRequested = false;
}

void Renderer::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
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

VulkanImage Renderer::CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usageFlags)
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

VulkanImage Renderer::CreateAndFillImage(TextureLoader::TextureResult* textureData, VkImageUsageFlags usageFlags)
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

void Renderer::DestroyImage(VulkanImage image)
{
    vkDestroyImageView(_vulkanContext.device, image.imageView, nullptr);
    vkDestroyImage(_vulkanContext.device, image.image, nullptr);
}

VulkanBuffer Renderer::CreateBuffer(size_t allocSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage)
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

void Renderer::DestroyBuffer(const VulkanBuffer& buffer)
{
    vmaDestroyBuffer(_allocator, buffer.buffer, buffer.allocation);
}

void Renderer::Draw()
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
        _window->bResizeRequested = true;
        return;
    }

    // Take and reset the cmd
    VkCommandBuffer cmd = GetCurrentFrame().commandBuffer;
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    // Start the cmd record
    VkCommandBufferBeginInfo cmdBeginInfo = VkHelpers::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    VkHelpers::TransitionImage(cmd, _previewImage.resource.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    // Create the common Buffer and the descriptorSet for it in the Per Frame Resources also fill it
    VulkanBuffer commonValuesBuffer = CreateBuffer(sizeof(float) * 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    GetCurrentFrame().deletionStack.Push([this, commonValuesBuffer]()
    {
        DestroyBuffer(commonValuesBuffer);
    });

    {
        float* commonValues;
        vmaMapMemory(_allocator, commonValuesBuffer.allocation, (void**)&commonValues);
        commonValues[0] = _previewImage.resource.imageExtent.width;
        commonValues[1] = _previewImage.resource.imageExtent.height;
        commonValues[2] = _sourceImage.resource.imageExtent.width;
        commonValues[3] = _sourceImage.resource.imageExtent.height;
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

    vkCmdDispatch(cmd, std::ceil((float)_previewImage.resource.imageExtent.width / 16.0f),
        std::ceil((float)_previewImage.resource.imageExtent.height / 16.0f), 1);
    // END DRAW ITSELF
    
    // EDITOR
    VkHelpers::TransitionImage(cmd, _previewImage.resource.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VkHelpers::TransitionImage(cmd, _swapchain.swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    DrawEditor(cmd, _swapchain.swapchainImageViews[swapchainImageIndex]);
    
    VkHelpers::TransitionImage(cmd, _swapchain.swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

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
        _window->bResizeRequested = true;
    }
    
    _frameNumber++;
}

void Renderer::DrawEditor(VkCommandBuffer cmd, VkImageView targetImageView)
{
    VkRenderingAttachmentInfo colorAttachment = VkHelpers::AttachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = VkHelpers::RenderingInfo(_swapchain.swapchainExtent, &colorAttachment);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();

    EditorContext ctx;
    ctx.sourceImage = _sourceImage;
    ctx.previewImage = _previewImage;

    Editor::BuildEditor(ctx);
    
    ImGui::Render();
    
    vkCmdBeginRendering(cmd, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vkCmdEndRendering(cmd);
}
