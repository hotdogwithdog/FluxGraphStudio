#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>
#include "Logger/Logger.h"


#define VK_CHECK(x)                                                     \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
            Logger::Log(Logger::LogLevel::Error, "Detected Vulkan error: " + std::string(string_VkResult(err))); \
            abort();                                                    \
        }                                                               \
    } while (0)