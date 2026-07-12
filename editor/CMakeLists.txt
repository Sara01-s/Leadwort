cmake_minimum_required(VERSION 3.20)
project(LeadwortEditorLib)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

# ImGui
set(IMGUI_DIR thirdparty/imgui)
add_library(imgui
        STATIC
            ${IMGUI_DIR}/imgui.cpp
            ${IMGUI_DIR}/imgui_draw.cpp
            ${IMGUI_DIR}/imgui_tables.cpp
            ${IMGUI_DIR}/imgui_widgets.cpp
            ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
            ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
target_compile_definitions(imgui PUBLIC IMGUI_ENABLE_DOCKING)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS "src/*.h" "src/*.hpp")

add_library(LeadwortEditorLib
        ${ENGINE_SOURCES}
        ${ENGINE_HEADERS}
)

target_include_directories(LeadwortEditorLib
        PUBLIC src
)

target_link_libraries(LeadwortEditorLib
        PUBLIC
            imgui

        PRIVATE
            LeadwortEngineLib
)

target_precompile_headers(LeadwortEditorLib PRIVATE pch.h)
