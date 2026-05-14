# =========================
# Libraries with CMake
# =========================

add_subdirectory(SDL)
add_subdirectory(vk-bootstrap)
add_subdirectory(glm)

# =========================
# ImGui (manual)
# =========================
add_library(imgui STATIC)

target_sources(imgui PRIVATE
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp

    imgui/backends/imgui_impl_sdl3.cpp
    imgui/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui PUBLIC imgui)

target_link_libraries(imgui PUBLIC
    Vulkan::Vulkan
    SDL3::SDL3
)

# =========================
# Header-only libs
# =========================

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

add_library(vma INTERFACE)
target_include_directories(vma INTERFACE vma)