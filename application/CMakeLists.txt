cmake_minimum_required(VERSION 3.20)
project(Leadwort)

add_executable(Leadwort src/main.cpp)

target_link_libraries(Leadwort
        PRIVATE LeadwortEngineLib
        PRIVATE LeadwortEditorLib
)

set_target_properties(Leadwort PROPERTIES VS_USER_PROPS_CXX "${VCPKG_ROOT}/scripts/buildsystems/msbuild/vcpkg.targets")
set(VCPKG_APPLOCAL_DEPS ON)

add_custom_command(TARGET Leadwort
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:Leadwort>/engine-assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/engine/engine-assets"
        "$<TARGET_FILE_DIR:Leadwort>/engine-assets"
)

if(WIN32)
    add_custom_command(TARGET Leadwort
            POST_BUILD
            COMMAND if not exist "$<TARGET_FILE_DIR:Leadwort>/engine-assets"
            cmd /c mklink /J "$<TARGET_FILE_DIR:Leadwort>/engine-assets" "${CMAKE_SOURCE_DIR}/engine/engine-assets"
    )
else()
    add_custom_command(TARGET Leadwort
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            "${CMAKE_SOURCE_DIR}/engine/engine-assets"
            "$<TARGET_FILE_DIR:Leadwort>/engine-assets"
    )
endif()

if(MINGW)
    target_link_options(Leadwort PRIVATE -static-libgcc -static-libstdc++)
endif()