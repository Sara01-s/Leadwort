cmake_minimum_required(VERSION 3.20)
project(GLEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(glad STATIC thirdparty/glad/glad.c)
add_library(stb STATIC thirdparty/stb/stb_image.cpp)
target_include_directories(glad PUBLIC thirdparty)
target_include_directories(stb PUBLIC thirdparty)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h" "src/*.hpp")

add_executable(GLEngine ${SOURCES} ${HEADERS})

target_compile_definitions(GLEngine PRIVATE
        ENGINE_ASSET_ROOT="engine-assets"
        GAME_ASSET_ROOT="game-assets"
        NOMINMAX
        GLFW_INCLUDE_NONE
)

find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

set(IMGUI_DIR thirdparty/imgui)
add_library(imgui STATIC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
target_compile_definitions(imgui PUBLIC IMGUI_ENABLE_DOCKING)

target_link_libraries(GLEngine PRIVATE
        glad
        stb
        glfw
        assimp::assimp
        glm::glm
        OpenAL::OpenAL
        OpenGL::GL
        imgui
)

target_include_directories(GLEngine PRIVATE include src thirdparty)

add_custom_command(TARGET GLEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/engine/engine-assets"
        "$<TARGET_FILE_DIR:GLEngine>/engine-assets"
)

if(WIN32)
    target_sources(GLEngine PRIVATE resources.rc)
endif()