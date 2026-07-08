cmake_minimum_required(VERSION 3.20)
project(LeadwortEditorLib)

add_library(LeadwortEditorLib
        src/public/EditorLayer.h
        src/public/ImGuiTheme.h
)

target_include_directories(LeadwortEditorLib
        PUBLIC src/public
)

target_link_libraries(LeadwortEditorLib
        PRIVATE LeadwortEngineLib
)