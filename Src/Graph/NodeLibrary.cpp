#include "NodeLibrary.h"

#include "DescriptionNodeInstance.h"
#include "RGNode.h"
#include "RGNodeInfo.h"
#include "ShaderCompiler/ShaderCompiler.h"

NodeLibrary::NodeLibrary(VulkanContext* vulkanContext)
{
    _vulkanContext = vulkanContext;
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

    VkDescriptorSetLayout descriptorSetLayout;
    {
        Descriptors::DescriptorLayoutBuilder builder;
        info->FillDescriptorSetLayoutBuilder(builder);
        descriptorSetLayout = builder.Build(_vulkanContext->device);
    }

    // Vulkan Pipeline (For that we need to compile the shader and do the includes)
    Graph::ParameterBindingLayout const * bindingInfo = info->GetParameterBindingLayout();

    info->GenerateFinalCode();

    ShaderCompiler::ShaderCompilationResult result = ShaderCompiler::CompileGlslCodeIntoSpirV(info->finalCode);
    if (result.bSuccess == false)
    {
        Logger::Log(Logger::LogLevel::Error, std::format("NodeLibrary::InitializeNode: The node with hanldle {} has failed, with error message: {}", handle, result.errorMessage));
        return;
    }

    
    
    
}
