cmake_minimum_required(VERSION 3.25)

project(FGS)

set(CMAKE_CXX_STANDARD 20)


# =========================
# VULKAN
# =========================
find_package(Vulkan REQUIRED)


# =========================
# OUTPUT DIRECTORY (IMPORTANT)
# =========================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/Release)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# =========================
# SOURCE
# =========================
file(GLOB_RECURSE FGS_SRC CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/Src/*.cpp
    ${CMAKE_SOURCE_DIR}/Src/*.h
)

add_executable(FGS ${FGS_SRC})

target_include_directories(FGS PRIVATE ${CMAKE_SOURCE_DIR}/Src)

# =========================
# THIRD PARTY
# =========================
add_subdirectory(ThirdParty)


target_link_libraries(FGS PRIVATE
	Vulkan::Vulkan
	SDL3::SDL3
	glm
	vk-bootstrap
	imgui
	stb
	vma
)


