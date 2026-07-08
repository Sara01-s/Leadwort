cmake_minimum_required(VERSION 3.20)
project(Leadwort)

add_executable(Leadwort src/main.cpp)

target_link_libraries(Leadwort
        PRIVATE LeadwortEngineLib
        PRIVATE LeadwortEditorLib
)

add_custom_command(TARGET Leadwort
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:Leadwort>/engine-assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/engine/src/engine-assets"
            "$<TARGET_FILE_DIR:Leadwort>/engine-assets"
)

if(MINGW)
    target_link_options(Leadwort PRIVATE -static-libgcc -static-libstdc++ -static -lwinpthread)
endif()