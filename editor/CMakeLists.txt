cmake_minimum_required(VERSION 3.20)
project(LeadwortEditorLib)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(imguizmo REQUIRED)
find_package(nfd CONFIG REQUIRED)

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
            imgui::imgui
            imguizmo::imguizmo
            nfd::nfd

        PRIVATE
            LeadwortEngineLib
)

target_precompile_headers(LeadwortEditorLib PRIVATE pch.h)
