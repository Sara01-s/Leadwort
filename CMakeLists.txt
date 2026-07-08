cmake_minimum_required(VERSION 3.20)
set(VCPKG_MANIFEST_MODE ON CACHE BOOL "" FORCE)
project(Leadwort VERSION 1.0.0 LANGUAGES CXX C)
    set(VCPKG_APPLOCAL_DEPS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(engine)
add_subdirectory(editor)
add_subdirectory(application)
