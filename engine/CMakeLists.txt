cmake_minimum_required(VERSION 3.20)
project(LeadwortEngineLib)

find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

# Glad
add_library(glad STATIC thirdparty/glad/glad.c)
target_include_directories(glad PUBLIC thirdparty)

# Stb
add_library(stb STATIC thirdparty/stb/stb_image.cpp)
target_include_directories(stb PUBLIC thirdparty)

# TinyEXR
add_library(tinyEXR STATIC thirdparty/tinyexr/tinyexr.cpp)
target_include_directories(tinyEXR PUBLIC thirdparty)
target_link_libraries(tinyEXR PRIVATE ZLIB::ZLIB)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS "src/*.h" "src/*.hpp")

add_library(LeadwortEngineLib
        STATIC
            ${ENGINE_SOURCES}
            ${ENGINE_HEADERS}
)

target_include_directories(LeadwortEngineLib
        PUBLIC
            src
            thirdparty
)

target_compile_definitions(LeadwortEngineLib
        PUBLIC
            ENGINE_ASSET_ROOT="engine-assets"
            GAME_ASSET_ROOT="game-assets"
            PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
            GLFW_INCLUDE_NONE
)

target_link_libraries(LeadwortEngineLib
        PRIVATE
            glad
            stb
            tinyEXR
            glfw
            assimp::assimp
            glm::glm
            OpenAL::OpenAL
            OpenGL::GL
            nlohmann_json::nlohmann_json
)

target_precompile_headers(LeadwortEngineLib PRIVATE pch.h)

enable_testing()
add_subdirectory(tests)