#pragma once
#include <imgui.h>

inline void SetupImGuiStyle() {
    // Light blue Comfy style - Modified from RegularLunar's ImThemes
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.1f;
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.WindowRounding = 10.0f;
    style.WindowBorderSize = 0.0f;
    style.WindowMinSize = ImVec2(30.0f, 30.0f);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    style.ChildRounding = 5.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 10.0f;
    style.PopupBorderSize = 0.0f;
    style.FramePadding = ImVec2(5.0f, 3.5f);
    style.FrameRounding = 5.0f;
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(5.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(5.0f, 5.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);
    style.IndentSpacing = 5.0f;
    style.ColumnsMinSpacing = 5.0f;
    style.ScrollbarSize = 15.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 15.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 5.0f;
    style.TabBorderSize = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	constexpr auto lightBluePrimary { ImVec4(0.20f, 0.65f, 1.00f, 0.60f) };
	constexpr auto lightBlueHover   { ImVec4(0.25f, 0.72f, 1.00f, 0.80f) };
	constexpr auto lightBlueActive  { ImVec4(0.15f, 0.58f, 0.90f, 1.00f) };

    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(1.0f, 1.0f, 1.0f, 0.360515f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09803922f, 0.09803922f, 0.09803922f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.09803922f, 0.09803922f, 0.09803922f, 1.0f);
    style.Colors[ImGuiCol_Border] = lightBluePrimary;
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.40f, 0.60f, 0.55f);
    style.Colors[ImGuiCol_FrameBgActive] = lightBluePrimary;
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.09803922f, 0.09803922f, 0.09803922f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.09803922f, 0.09803922f, 0.09803922f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.25882354f, 0.25882354f, 0.25882354f, 0.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 0.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.23529412f, 0.23529412f, 0.23529412f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.29411766f, 0.29411766f, 0.29411766f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = lightBlueActive;
    style.Colors[ImGuiCol_SliderGrab] = lightBluePrimary;
    style.Colors[ImGuiCol_SliderGrabActive] = lightBlueActive;
    style.Colors[ImGuiCol_Button] = lightBluePrimary;
    style.Colors[ImGuiCol_ButtonHovered] = lightBlueHover;
    style.Colors[ImGuiCol_ButtonActive] = lightBlueActive;
    style.Colors[ImGuiCol_Header] = lightBluePrimary;
    style.Colors[ImGuiCol_HeaderHovered] = lightBlueHover;
    style.Colors[ImGuiCol_HeaderActive] = lightBlueActive;
    style.Colors[ImGuiCol_Separator] = lightBluePrimary;
    style.Colors[ImGuiCol_SeparatorHovered] = lightBlueHover;
    style.Colors[ImGuiCol_SeparatorActive] = lightBlueActive;
    style.Colors[ImGuiCol_ResizeGrip] = lightBluePrimary;
    style.Colors[ImGuiCol_ResizeGripHovered] = lightBlueHover;
    style.Colors[ImGuiCol_ResizeGripActive] = lightBlueActive;
    style.Colors[ImGuiCol_Tab] = lightBluePrimary;
    style.Colors[ImGuiCol_TabHovered] = lightBlueHover;
    style.Colors[ImGuiCol_TabActive] = lightBlueActive;
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0f, 0.4509804f, 1.0f, 0.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13333334f, 0.25882354f, 0.42352942f, 0.0f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.29411766f, 0.29411766f, 0.29411766f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = lightBlueHover;
    style.Colors[ImGuiCol_PlotHistogram] = lightBluePrimary;
    style.Colors[ImGuiCol_PlotHistogramHovered] = lightBlueHover;
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882353f, 0.1882353f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] = lightBluePrimary;
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.20f, 0.65f, 1.00f, 0.30f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.03433478f);
    style.Colors[ImGuiCol_TextSelectedBg] = lightBluePrimary;
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.9f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
}