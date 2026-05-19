#pragma once

#include <deque>
#include <span>

#include "VkTypes.h"
#include <vector>

namespace Descriptors
{
    struct DescriptorLayoutBuilder
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        void AddBinding(uint32_t binding, VkDescriptorType type);
        void Clear();

        VkDescriptorSetLayout Build(VkDevice device, VkShaderStageFlags shaderStages = VK_SHADER_STAGE_COMPUTE_BIT, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
    };

    struct PoolSizeRatio
    {
        VkDescriptorType type;
        uint32_t ratio;
    };

    class DescriptorAllocator
    {
    public:
        VkDescriptorPool pool;

    public:
        DescriptorAllocator() = default;

        void InitPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios); // Span is more like a linked list so with this we are not coping the list
        void ClearPool(VkDevice device);
        void DestroyPool(VkDevice device);

        VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout);
    };


    class DescriptorWriter
    {
    public:
        DescriptorWriter() = default;

        void WriteImage(int binding, VkImageView imageView, VkSampler sampler, VkImageLayout layout, VkDescriptorType type);
        void WriteBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);

        void Clear();
        void UpdateSet(VkDevice device, VkDescriptorSet set);
    
    private:
        // This are std::deque because they are dual linked lists so it's safe to take pointers to his elements
        std::deque<VkDescriptorImageInfo> _imageInfos;
        std::deque<VkDescriptorBufferInfo> _bufferInfos;
        std::vector<VkWriteDescriptorSet> _writes;
    };
    
}

