#pragma once
#include <imgui.h>


class Editor
{
public:
    Editor() = default;

    static void BuildEditor(ImTextureID& texture, float width, float height);
};
