#pragma once

#include "ImGuiTheme.h"
#include "engine/asset-management/public/AssetManager.h"
#include "engine/core/public/Application.h"
#include "engine/ui/UILayer.h"
#include "engine/utils/public/Logger.h"

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <string>

namespace Engine::Editor {

class EditorLayer final : public UI::UILayer {
public:
    void Init(const std::uint64_t windowHandle) override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    	io.IniFilename = "editor.ini";

        SetupImGuiStyle();

    	auto const& fontPath = AssetManagement::EngineAssets::Resolve("fonts/font_inter_variable.ttf");
    	io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 24.0f);
    	io.FontDefault = io.Fonts->Fonts[0];

		const auto window = reinterpret_cast<GLFWwindow*>(windowHandle);
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 430");
    }

    void StartFrame() override {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void EndFrame() override {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backupContext = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupContext);
        }
    }

	static void SetupDockSpace() {
        static bool firstTime = true;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

		constexpr ImGuiWindowFlags windowFlags =
			ImGuiWindowFlags_MenuBar
    		| ImGuiWindowFlags_NoDocking
    		| ImGuiWindowFlags_NoTitleBar
    		| ImGuiWindowFlags_NoCollapse
    		| ImGuiWindowFlags_NoResize
    		| ImGuiWindowFlags_NoMove
    		| ImGuiWindowFlags_NoBringToFrontOnFocus
    		| ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("DockSpace", nullptr, windowFlags);
		const ImGuiID dockSpaceId = ImGui::GetID("MyDockSpace");

    	if (!ImGui::DockBuilderGetNode(dockSpaceId)) {
    		ImGui::DockBuilderRemoveNode(dockSpaceId);
    		ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
    		ImGui::DockBuilderSetNodeSize(dockSpaceId, viewport->WorkSize);

    		ImGuiID center = dockSpaceId;
			const ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.2f, nullptr, &center);

    		ImGui::DockBuilderDockWindow("Scene", center);
    		ImGui::DockBuilderDockWindow("Game", center);
    		ImGui::DockBuilderDockWindow("Status", left);
    		ImGui::DockBuilderFinish(dockSpaceId);
    	}

        ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        ImGui::End();
    }

    ~EditorLayer() override {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
};

}