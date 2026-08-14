#pragma once

#include "ImGuiTheme.h"
#include "Leadwort/systems/public/Input.h"

#include <Leadwort/asset-management/public/AssetDatabase.h>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <string>

namespace Editor::Core  {

	class EditorCore {
	public:
		static void Initialize(const std::uint64_t windowHandle) {
	        IMGUI_CHECKVERSION();
	        ImGui::CreateContext();

	        ImGuiIO& io { ImGui::GetIO() };
	        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    		io.IniFilename = "editor.ini";

	        SetupImGuiStyle();

    		auto const& fontPath { Leadwort::AssetManagement::EngineAssets::ResolvePath("fonts/font_inter_variable.ttf") };
    		io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 24.0f);
    		io.FontDefault = io.Fonts->Fonts[0];

			const auto window { reinterpret_cast<GLFWwindow*>(windowHandle) };
	        ImGui_ImplGlfw_InitForOpenGL(window, true);
	        ImGui_ImplOpenGL3_Init("#version 430");
	    }

		static void StartFrame() {
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();
		}

		static void EndFrame() {
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
				GLFWwindow* backupContext { glfwGetCurrentContext() };
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backupContext);
			}
		}

		static void SetupDockSpace() {
	        const ImGuiViewport* viewport = ImGui::GetMainViewport();
	        ImGui::SetNextWindowPos(viewport->WorkPos);
	        ImGui::SetNextWindowSize(viewport->WorkSize);
	        ImGui::SetNextWindowViewport(viewport->ID);

			constexpr ImGuiWindowFlags windowFlags {
				ImGuiWindowFlags_MenuBar
    			| ImGuiWindowFlags_NoDocking
    			| ImGuiWindowFlags_NoTitleBar
    			| ImGuiWindowFlags_NoCollapse
    			| ImGuiWindowFlags_NoResize
    			| ImGuiWindowFlags_NoMove
    			| ImGuiWindowFlags_NoBringToFrontOnFocus
    			| ImGuiWindowFlags_NoNavFocus
			};

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

	    ~EditorCore() {
	        ImGui_ImplOpenGL3_Shutdown();
	        ImGui_ImplGlfw_Shutdown();
	        ImGui::DestroyContext();
	    }
	};

}