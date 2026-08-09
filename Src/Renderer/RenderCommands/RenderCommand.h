#pragma once

#include "GPU/VkTypes.h"


class RenderCommand
{
public:
    virtual ~RenderCommand() = default;
    
    virtual void Execute(const VkCommandBuffer& cmd) const = 0;
};
