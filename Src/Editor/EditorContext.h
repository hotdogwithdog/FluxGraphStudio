#pragma once
#include <imgui.h>

#include "GPU/VulkanImage.h"


struct EditorImage
{
    VulkanImage resource;
    ImTextureID ID;
};


struct EditorContext
{
    EditorImage sourceImage;
    EditorImage previewImage;
    // TODO: Expand with EditorGraph
};
