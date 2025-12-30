#pragma once

#include "imgui.h"
#include "rlImGui.h"

inline void SetEditorTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(6, 3);
    style.CellPadding = ImVec2(6, 3);
    style.ItemSpacing = ImVec2(6, 4);
    style.ItemInnerSpacing = ImVec2(4, 4);
    style.TouchExtraPadding = ImVec2(0, 0);
    style.IndentSpacing = 18;
    style.ScrollbarSize = 14;
    style.GrabMinSize = 8;

    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 0;
    style.TabBorderSize = 0;
    style.WindowRounding = 0;  
    style.ChildRounding = 0;
    style.FrameRounding = 2;
    style.PopupRounding = 2;
    style.ScrollbarRounding = 3;
    style.GrabRounding = 2;
    style.LogSliderDeadzone = 4;
    style.TabRounding = 2;

    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);  
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.5f);
    
    colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.12f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    
    colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    
    colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.06f, 0.06f, 0.06f, 0.75f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    
    colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.60f, 0.20f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.90f, 0.55f, 0.20f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.65f, 0.25f, 1.00f);
    
    colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.90f, 0.55f, 0.20f, 0.60f);
    colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.60f, 0.20f, 0.80f);
    
    colors[ImGuiCol_Header] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.90f, 0.55f, 0.20f, 0.40f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.90f, 0.55f, 0.20f, 0.60f);
    
    colors[ImGuiCol_Separator] = ImVec4(0.22f, 0.22f, 0.24f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.90f, 0.55f, 0.20f, 0.60f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.90f, 0.55f, 0.20f, 0.80f);
    
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.90f, 0.55f, 0.20f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.90f, 0.55f, 0.20f, 0.60f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 0.60f, 0.20f, 0.90f);
    
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.90f, 0.55f, 0.20f, 0.60f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.55f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.55f, 0.20f, 0.70f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.20f, 1.00f);
    
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);
    
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.90f, 0.55f, 0.20f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.60f, 0.20f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.90f, 0.55f, 0.20f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
    
    style.AntiAliasedLines = true;
    style.AntiAliasedFill = true;
    style.CurveTessellationTol = 1.25f;
}

inline void LoadEditorFonts() {
    ImGuiIO& io = ImGui::GetIO();
    
    io.Fonts->Clear();
    
    ImFontConfig font_config;
    font_config.OversampleH = 1;  
    font_config.OversampleV = 1;
    font_config.PixelSnapH = true;  
    font_config.RasterizerDensity = 1.0f;  
    
    io.Fonts->AddFontDefault();
    io.Fonts->Build();
    
    rlImGuiReloadFonts();
}
