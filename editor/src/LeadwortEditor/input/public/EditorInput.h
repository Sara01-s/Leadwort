#pragma once
#include "Leadwort/systems/public/Input.h"

#include <imgui.h>
#include <ImGuizmo.h>
#include <imgui_internal.h>
#include <utility>

namespace Editor::Input {

class EditorInput {
public:
	void Update() {
		const bool overGizmo { ImGuizmo::IsOver() || ImGuizmo::IsUsing() };
		Leadwort::Systems::Input::Mouse::SetUIHovered(ImGui::GetIO().WantCaptureMouse || overGizmo);

		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z)) {
			m_UndoRequested = true;
		}

		if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
			m_DeleteRequested = true;
		}

		if (ImGui::IsKeyPressed(ImGuiKey_W)) { m_GizmoOperation = ImGuizmo::TRANSLATE; }
		if (ImGui::IsKeyPressed(ImGuiKey_E)) { m_GizmoOperation = ImGuizmo::ROTATE; }
		if (ImGui::IsKeyPressed(ImGuiKey_R)) { m_GizmoOperation = ImGuizmo::SCALE; }
	}

	[[nodiscard]] bool UndoRequestPending() noexcept { return std::exchange(m_UndoRequested, false); }
	[[nodiscard]] bool DeleteRequestPending() noexcept { return std::exchange(m_DeleteRequested, false); }
	[[nodiscard]] ImGuizmo::OPERATION GetGizmoOperation() const noexcept { return m_GizmoOperation; }

private:
	bool m_UndoRequested { false };
	bool m_DeleteRequested { false };
	ImGuizmo::OPERATION m_GizmoOperation { ImGuizmo::TRANSLATE };
};

}