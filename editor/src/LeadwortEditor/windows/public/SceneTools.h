#pragma once
#include "LeadwortEditor/data/EditorContext.h"
#include "imgui.h"

#include <Leadwort/systems/public/Input.h>

namespace Editor::Windows {

	class SceneTools final {
	public:
		explicit SceneTools(EditorContext& editorContext) : m_EditorContext(editorContext) {}

		[[nodiscard]] bool IsHovered() const noexcept { return m_Hovered; }

		void Draw(const ImVec2& viewportPos, const bool acceptsShortcuts) {
			if (acceptsShortcuts) {
				HandleShortcuts();
			}

			ImGui::SetCursorScreenPos(ImVec2(viewportPos.x + kMargin, viewportPos.y + kMargin));

			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
			ImGui::PushStyleColor(ImGuiCol_ChildBg, kBackgroundColor);

			constexpr ImGuiChildFlags childFlags {
				ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY
			};

			constexpr ImGuiWindowFlags windowFlags {
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings
			};

			m_Hovered = false;

			if (ImGui::BeginChild("##scene_tools", ImVec2(kWidth, 0.0f), childFlags, windowFlags)) {
				DrawOperationTabs();
				ImGui::Separator();
				DrawModeSelector();

				m_Hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
			}

			ImGui::EndChild();

			ImGui::PopStyleColor();
			ImGui::PopStyleVar(2);
		}

	private:
		void HandleShortcuts() const {
			if (Leadwort::Systems::Input::Mouse::IsCaptured() || ImGui::GetIO().WantTextInput) {
				return;
			}

			if (ImGui::IsKeyPressed(ImGuiKey_W, false)) {
				m_EditorContext.GizmoOperation = ImGuizmo::TRANSLATE;
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_E, false)) {
				m_EditorContext.GizmoOperation = ImGuizmo::ROTATE;
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_R, false)) {
				m_EditorContext.GizmoOperation = ImGuizmo::SCALE;
			}
		}

		void DrawOperationTabs() {
			if (!ImGui::BeginTabBar("##gizmo_operation", ImGuiTabBarFlags_None)) {
				return;
			}

			const bool syncSelection { !m_SelectionSynced || m_SyncedOperation != m_EditorContext.GizmoOperation };

			DrawOperationTab("Translate", "W", ImGuizmo::TRANSLATE, syncSelection);
			DrawOperationTab("Rotate", "E", ImGuizmo::ROTATE, syncSelection);
			DrawOperationTab("Scale", "R", ImGuizmo::SCALE, syncSelection);

			ImGui::EndTabBar();

			m_SyncedOperation = m_EditorContext.GizmoOperation;
			m_SelectionSynced = true;
		}

		void DrawOperationTab(const char* label, const char* shortcut, const ImGuizmo::OPERATION operation, const bool syncSelection) const {
			const bool isSelected { m_EditorContext.GizmoOperation == operation };

			const ImGuiTabItemFlags flags {
				syncSelection && isSelected ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None
			};

			const bool opened { ImGui::BeginTabItem(label, nullptr, flags) };

			const bool clicked { ImGui::IsItemClicked() };

			ImGui::SetItemTooltip("%s (%s)", label, shortcut);

			if (opened) {
				ImGui::EndTabItem();
			}

			if (clicked) {
				m_EditorContext.GizmoOperation = operation;
			}
		}

		void DrawModeSelector() const {
			if (ImGui::RadioButton("Local", m_EditorContext.GizmoMode == ImGuizmo::LOCAL)) {
				m_EditorContext.GizmoMode = ImGuizmo::LOCAL;
			}

			ImGui::SameLine();

			if (ImGui::RadioButton("World", m_EditorContext.GizmoMode == ImGuizmo::WORLD)) {
				m_EditorContext.GizmoMode = ImGuizmo::WORLD;
			}
		}

	private:
		static constexpr float kMargin { 10.0f };
		static constexpr float kWidth { 232.0f * 1.5f };

		static constexpr ImVec4 kBackgroundColor { 0.10f, 0.10f, 0.11f, 0.88f };

	private:
		EditorContext& m_EditorContext;

		bool m_Hovered { false };

		ImGuizmo::OPERATION m_SyncedOperation { ImGuizmo::TRANSLATE };
		bool m_SelectionSynced { false };
	};

}
