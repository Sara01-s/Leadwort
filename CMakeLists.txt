cmake_minimum_required(VERSION 3.20)
project(Leadwort VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(efsw CONFIG REQUIRED)

# Glad
add_library(glad STATIC thirdparty/glad/glad.c)
target_include_directories(glad PUBLIC thirdparty)

# Stb
add_library(stb STATIC thirdparty/stb/stb_image.cpp)
target_include_directories(stb PUBLIC thirdparty)

# ImGui
set(IMGUI_DIR thirdparty/imgui)
add_library(imgui STATIC
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

file(GLOB_RECURSE ENGINE_SOURCES "src/*.cpp")
file(GLOB_RECURSE ENGINE_HEADERS "src/*.h" "src/*.hpp")

list(REMOVE_ITEM ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_library(LeadwortLib STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

target_include_directories(LeadwortLib PUBLIC include src thirdparty)

target_compile_definitions(LeadwortLib PUBLIC
        ENGINE_ASSET_ROOT="engine-assets"
        GAME_ASSET_ROOT="game-assets"
        PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        GLFW_INCLUDE_NONE
)

target_link_libraries(LeadwortLib PUBLIC
        glad
        stb
        imgui
        glfw
        assimp::assimp
        glm::glm
        OpenAL::OpenAL
        OpenGL::GL
        efsw::efsw
)

target_precompile_headers(LeadwortLib PRIVATE pch.h)

add_executable(Leadwort src/main.cpp)

target_link_libraries(Leadwort PRIVATE LeadwortLib)

if(WIN32)
    target_sources(Leadwort PRIVATE resources.rc)
endif()

add_custom_command(TARGET Leadwort POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/engine/engine-assets"
        "$<TARGET_FILE_DIR:Leadwort>/engine-assets"
)

enable_testing()
add_subdirectory(tests)