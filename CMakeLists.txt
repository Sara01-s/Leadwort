cmake_minimum_required(VERSION 3.20)
project(GLEngine VERSION 1.0.0 LANGUAGES CXX)

# --- 1. Project Configuration ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Configurable options
option(ENABLE_ASAN "Enable AddressSanitizer" ON)

# --- 2. External Dependencies ---
find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# --- 3. Third-party Libraries (Local) ---
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

# --- 4. Executable Definition ---
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h" "src/*.hpp")

add_executable(GLEngine ${SOURCES} ${HEADERS})

# --- 5. GLEngine Configuration ---
target_include_directories(GLEngine PRIVATE include src thirdparty)

target_compile_definitions(GLEngine PRIVATE
        ENGINE_ASSET_ROOT="engine-assets"
        GAME_ASSET_ROOT="game-assets"
        PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        GLFW_INCLUDE_NONE
)

target_link_libraries(GLEngine PRIVATE
        glad
        stb
        imgui
        glfw
        assimp::assimp
        glm::glm
        OpenAL::OpenAL
        OpenGL::GL
)

target_precompile_headers(GLEngine PRIVATE pch.h)

# --- 6. Platform-Specific Configuration ---
if(WIN32)
    target_sources(GLEngine PRIVATE resources.rc)
endif()

# Sanitizers (Toggleable option)
if(ENABLE_ASAN)
    if(MSVC)
        target_compile_options(GLEngine PRIVATE /fsanitize=address)
    else()
        target_compile_options(GLEngine PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(GLEngine PRIVATE -fsanitize=address)
    endif()
endif()

# --- 7. Post-Build Tasks ---
add_custom_command(TARGET GLEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/engine/engine-assets"
        "$<TARGET_FILE_DIR:GLEngine>/engine-assets"
)