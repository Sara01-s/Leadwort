cmake_minimum_required(VERSION 3.20)
project(LeadwortEngineLib)

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
            glfw
            assimp::assimp
            glm::glm
            OpenAL::OpenAL
            OpenGL::GL
            efsw::efsw
)

target_precompile_headers(LeadwortEngineLib PRIVATE pch.h)

if(WIN32)
    target_sources(LeadwortEngineLib PRIVATE resources.rc)
endif()

enable_testing()
add_subdirectory(tests)