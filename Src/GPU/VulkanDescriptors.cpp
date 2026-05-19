#include "VulkanDescriptors.h"

namespace Descriptors
{
#pragma region DescriptorLayoutBuilder
    void DescriptorLayoutBuilder::AddBinding(uint32_t binding, VkDescriptorType type)
    {
        bindings.emplace_back(binding, type, 1, 0, nullptr);
    }

    void DescriptorLayoutBuilder::Clear()
    {
        bindings.clear();
    }

    VkDescriptorSetLayout DescriptorLayoutBuilder::Build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext, VkDescriptorSetLayoutCreateFlags flags)
    {
        for (VkDescriptorSetLayoutBinding& binding : bindings)
        {
            binding.stageFlags |= shaderStages;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .pNext = pNext };

        layoutInfo.pBindings = bindings.data();
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.flags = flags;

        VkDescriptorSetLayout set;
        VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &set));
        
        return set;
    }
#pragma endregion
    
#pragma region DescriptorAllocator
    void DescriptorAllocator::InitPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
    {
        std::vector<VkDescriptorPoolSize> poolSizes;
        poolSizes.reserve(poolRatios.size());

        for (PoolSizeRatio& ratio : poolRatios)
        {
            poolSizes.emplace_back(ratio.type, static_cast<uint32_t>(ratio.ratio * maxSets));
        }
        
        VkDescriptorPoolCreateInfo poolInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, .pNext = nullptr };
        poolInfo.flags = 0;
        poolInfo.maxSets = maxSets;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        
        VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool));
    }

    void DescriptorAllocator::ClearPool(VkDevice device)
    {
        VK_CHECK(vkResetDescriptorPool(device, pool, 0));
    }

    void DescriptorAllocator::DestroyPool(VkDevice device)
    {
        vkDestroyDescriptorPool(device, pool, nullptr);
    }

    VkDescriptorSet DescriptorAllocator::Allocate(VkDevice device, VkDescriptorSetLayout layout)
    {
        VkDescriptorSetAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, .pNext = nullptr };
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        VkDescriptorSet ds;
        VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &ds));

        return ds;
    }
#pragma endregion

#pragma region DescriptorWriter
    void DescriptorWriter::WriteImage(int binding, VkImageView imageView, VkSampler sampler, VkImageLayout layout, VkDescriptorType type)
    {
        VkDescriptorImageInfo& imageInfo = _imageInfos.emplace_back(sampler, imageView, layout);

        VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .pNext = nullptr };
        write.dstBinding = binding;
        write.dstSet = VK_NULL_HANDLE;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pImageInfo = &imageInfo;

        _writes.push_back(write);
    }

    void DescriptorWriter::WriteBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type)
    {
        VkDescriptorBufferInfo& bufferInfo = _bufferInfos.emplace_back(buffer, offset, size);

        VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        write.dstBinding = binding;
        write.dstSet = VK_NULL_HANDLE;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pBufferInfo = &bufferInfo;

        _writes.push_back(write);
    }

    void DescriptorWriter::Clear()
    {
        _imageInfos.clear();
        _writes.clear();
        _bufferInfos.clear();
    }

    void DescriptorWriter::UpdateSet(VkDevice device, VkDescriptorSet set)
    {
        for (VkWriteDescriptorSet& write : _writes)
        {
            write.dstSet = set;
        }

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(_writes.size()), _writes.data(), 0, nullptr);
    }
#pragma endregion
    
}

