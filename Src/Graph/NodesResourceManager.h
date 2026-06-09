#pragma once
#include <cstdint>

struct DescriptionNodeInstance;

using RGNodeHandle = uint32_t;
using NodeInstanceHandle = uint32_t;
using DescriptionNodeInstanceHandle = uint32_t;


namespace NodesResourceManager
{
    DescriptionNodeInstanceHandle RegisterDescriptionNodeInstanceHandle(DescriptionNodeInstance& descriptionNodeInstance);
    DescriptionNodeInstance* GetDescriptionNodeInstance(const DescriptionNodeInstanceHandle& descriptionNodeInstanceHandle);
    void DestroyDescriptionNodeInstance(const DescriptionNodeInstanceHandle& descriptionNodeInstanceHandle);
    
}
