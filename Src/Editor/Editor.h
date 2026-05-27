#pragma once



struct EditorImage;
struct EditorContext;

class Editor
{
public:
    Editor() = default;

    static void BuildEditor(const EditorContext& ctx);


private:
    static void BeginDockSpace();
    static void EndDockSpace();
    
    static void BuildImage(const EditorImage& image, const char* title = "");
};
