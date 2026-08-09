#include "NodeLibrary.h"

#include "DescriptionNodeInstance.h"
#include "RGNode.h"
#include "RGNodeInfo.h"
#include "ShaderCompiler/ShaderCompiler.h"

NodeLibrary::NodeLibrary(VulkanContext* vulkanContext, VkDescriptorSetLayout commonDescriptorSetLayout)
{
    _vulkanContext = vulkanContext;
    _commonDescriptorSetLayout = commonDescriptorSetLayout;
}

RGNode* NodeLibrary::GetNodeFromDescription(DescriptionNodeInstanceHandle handle)
{
    DescriptionNodeInstance* descriptionNodeInstance = NodesResourceManager::GetDescriptionNodeInstance(handle);
    if (descriptionNodeInstance == nullptr) return nullptr;
    RGNodeInfo* nodeInfo = AssetsManager::GetNodeInfo(descriptionNodeInstance->nodeInfoHandle);
    if (nodeInfo == nullptr) return nullptr;

    return GetNodeFromInfo(descriptionNodeInstance->nodeInfoHandle);
}

RGNode* NodeLibrary::GetNodeFromInfo(NodeInfoHandle handle)
{
    if (!AssetsUtils::IsValid(handle))
    {
        Logger::Log(Logger::LogLevel::Error, "NodeLibrary::GetNodeFromInfo: Node info handle is invalid");
        return nullptr;
    }

    RGNodeInfo* info = AssetsManager::GetNodeInfo(handle);
    
    if (!_nodes.contains(handle))
    {
        _nodes.emplace(handle, RGNode(handle, info->version));
        // Must initialize the GPUResources of the node (this is done here because the recompile of the version checks will destroy the previous resources and in here they do not exist)
        InitializeNode(handle);
    }

    RGNode* node = &_nodes[handle];
    
    
    if (node->version != info->version)
    {
        // TODO: Recompile the node with the nodeInfo, and update on the map with the same 
    }

    return node;
}

void NodeLibrary::InitializeNode(NodeInfoHandle handle)
{
    // Descriptors set layout
    RGNodeInfo* info = AssetsManager::GetNodeInfo(handle);
    if (info == nullptr) return;

    VkDescriptorSetLayout nodeDescriptorSetLayout;
    {
        Descriptors::DescriptorLayoutBuilder builder;
        info->FillDescriptorSetLayoutBuilder(builder);
        nodeDescriptorSetLayout = builder.Build(_vulkanContext->device);
    }

    // Vulkan Pipeline (For that we need to compile the shader and do the includes)
    Graph::ParameterBindingLayout const * bindingInfo = info->GetParameterBindingLayout();

    info->GenerateFinalCode();

    ShaderCompiler::ShaderCompilationResult compilerResult = ShaderCompiler::CompileGlslCodeIntoSpirV(info->finalCode);
    if (compilerResult.bSuccess == false)
    {
        Logger::Log(Logger::LogLevel::Error, std::format("NodeLibrary::InitializeNode: The node with hanldle {} has failed, with error message: {}", handle, compilerResult.errorMessage));
        return;
    }

    RGNode* node = &_nodes[handle];
    node->descriptorSetLayout = nodeDescriptorSetLayout;
    node->version = info->version;

    VkDescriptorSetLayout descriptorSetLayouts[] = { _commonDescriptorSetLayout, nodeDescriptorSetLayout };
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .pNext = nullptr };
    pipelineLayoutInfo.flags = 0;
    pipelineLayoutInfo.setLayoutCount = 2;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    VK_CHECK(vkCreatePipelineLayout(_vulkanContext->device, &pipelineLayoutInfo, nullptr, &node->pipeline.layout));

    VkShaderModule nodeShaderModule;
    if (!VkHelpers::LoadShaderModule(_vulkanContext->device, compilerResult.spirV, &nodeShaderModule))
    {
        Logger::Log(Logger::LogLevel::Error, std::format("NodeLibrary::InitializeNode: The Node [{}] has failed the creation of the shaderModule", info->name));
    }

    VkPipelineShaderStageCreateInfo stageInfo { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr };
    stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageInfo.module = nodeShaderModule;
    stageInfo.pName = "main";

    VkComputePipelineCreateInfo nodePipelineCreateInfo = { .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, .pNext = nullptr };
    nodePipelineCreateInfo.layout = node->pipeline.layout;
    nodePipelineCreateInfo.stage = stageInfo;

    VK_CHECK(vkCreateComputePipelines(_vulkanContext->device, VK_NULL_HANDLE, 1, &nodePipelineCreateInfo, nullptr, &node->pipeline.pipeline));
    
    vkDestroyShaderModule(_vulkanContext->device, nodeShaderModule, nullptr);
}
