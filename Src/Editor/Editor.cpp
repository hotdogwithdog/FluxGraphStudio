#include "Editor.h"

#include <imgui.h>

#include "EditorContext.h"

void Editor::BuildEditor(const EditorContext& ctx)
{
    ImGui::NewFrame();

    BeginDockSpace();
    
    BuildImage(ctx.sourceImage, "Original Image");
    BuildImage(ctx.previewImage, "Preview Image");

    EndDockSpace();
}

void Editor::BeginDockSpace()
{
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    windowFlags |= ImGuiWindowFlags_NoTitleBar;
    windowFlags |= ImGuiWindowFlags_NoCollapse;
    windowFlags |= ImGuiWindowFlags_NoResize;
    windowFlags |= ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    windowFlags |= ImGuiWindowFlags_NoNavFocus;
    windowFlags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("MainDockspace", nullptr, windowFlags);

    ImGui::PopStyleVar();

    ImGuiID dockspaceID = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
}

void Editor::EndDockSpace()
{
    ImGui::End();
}

void Editor::BuildImage(const EditorImage& image, const char* title /*= " "*/)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    if (ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 size = ImGui::GetContentRegionAvail();
        float sourceAspectRatio = (float)image.resource.imageExtent.width / (float)image.resource.imageExtent.height;
        if (size.x > size.y * sourceAspectRatio)
        {
            size.x = size.y * sourceAspectRatio;
        }
        else
        {
            size.y = size.x * (1.0f/sourceAspectRatio);
        }

        ImVec2 currentCursorPos = ImGui::GetCursorPos();
        ImVec2 cursorPos(currentCursorPos.x + (avail.x - size.x) * 0.5f, currentCursorPos.y + (avail.y - size.y) * 0.5f);
        
        ImGui::SetCursorPos(cursorPos);
        ImGui::Image(image.ID, size);
    }
    ImGui::End();

    ImGui::PopStyleVar();
}
