#pragma once
#include "LeadwortEditor/core/public/IEditorWindow.h"
#include "LeadwortEditor/data/EditorContext.h"
#include "imgui.h"

namespace Editor::Windows {

	class SceneTools final : public Core::IEditorWindow {
	public:
		explicit SceneTools(EditorContext& editorContext) : m_EditorContext(editorContext) {}

		std::string_view GetName() const noexcept override { return "Scene tools"; }

		void OnGuiRender() override {
			if (ImGui::BeginTabBar(GetName().data(), ImGuiTabBarFlags_None)) {

				if (ImGui::BeginTabItem("Translate")) {
					m_EditorContext.gizmoOperation = ImGuizmo::TRANSLATE;
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Rotate")) {
					m_EditorContext.gizmoOperation = ImGuizmo::ROTATE;
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Scale")) {
					m_EditorContext.gizmoOperation = ImGuizmo::SCALE;
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}

			ImGui::Separator();

			if (ImGui::RadioButton("Local", m_EditorContext.gizmoMode == ImGuizmo::LOCAL)) {
				m_EditorContext.gizmoMode = ImGuizmo::LOCAL;
			}

			ImGui::SameLine();

			if (ImGui::RadioButton("World", m_EditorContext.gizmoMode == ImGuizmo::WORLD)) {
				m_EditorContext.gizmoMode = ImGuizmo::WORLD;
			}
		}

	private:
		EditorContext& m_EditorContext;
	};

}