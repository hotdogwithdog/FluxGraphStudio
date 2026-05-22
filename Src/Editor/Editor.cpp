#include "Editor.h"

#include <imgui.h>

#include "GPU/GPUTypes.h"

void Editor::BuildEditor(ImTextureID& texture, float width, float height)
{
    ImGui::NewFrame();

    if (ImGui::Begin("OriginalImage"))
    {
        ImGui::Text("Original Image");
        
        ImGui::Image(texture, ImVec2(width, height));
        
        
    }
    ImGui::End();
}
