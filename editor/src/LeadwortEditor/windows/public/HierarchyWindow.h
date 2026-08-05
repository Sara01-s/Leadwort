#pragma once
#include "Leadwort/components/public/Transform.h"
#include "Leadwort/core/public/Entity.h"
#include "LeadwortEditor/core/public/IEditorWindow.h"
#include "LeadwortEditor/data/EditorContext.h"
#include "imgui.h"

namespace Editor::Windows {

class HierarchyWindow final : public Core::IEditorWindow {
public:
	explicit HierarchyWindow(EditorContext& editorContext) : m_EditorContext(editorContext) {}

	std::string_view GetName() const noexcept override {
		return "Hierarchy";
	}

	void OnGuiRender() override {
		ImGui::Begin(GetName().data());
		LW_ASSERT(m_EditorContext.openedScene != nullptr, "No scene opened");

		for (const auto& entity : m_EditorContext.openedScene->GetEntityMap() | std::views::values) {
			const bool hasParent { entity->GetTransform().HasParent() };
			const bool isChildOfRoot { hasParent && entity->GetTransform().GetParent()->GetEntity().GetID() == Leadwort::Core::Entity::ROOT_ENTITY_ID };

			if (!hasParent || isChildOfRoot) {
				DrawEntityNode(*entity);
			}
		}

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
			m_EditorContext.ClearSelection();
		}

		ImGui::End();
	}

private:
	void DrawEntityNode(const Leadwort::Core::Entity& entity) {
		ImGuiTreeNodeFlags flags {
			ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth
		};

		const bool hasChildren { entity.GetTransform().HasChildren() };
		if (!hasChildren) {
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

		const auto* selectedID = std::get_if<Leadwort::EntityID>(&m_EditorContext.selection);
		const bool isSelected = selectedID && *selectedID == entity.GetID();

		if (isSelected) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		if (isSelected) {
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.5f, 0.8f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.0f, 0.6f, 0.9f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.0f, 0.4f, 0.7f, 1.0f));
		}

		const bool opened { ImGui::TreeNodeEx(
			reinterpret_cast<void*>(static_cast<intptr_t>(entity.GetID())),
			flags,
			"%s", entity.name.data()
		) };

		if (isSelected) {
			ImGui::PopStyleColor(3);
		}

		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
			m_EditorContext.SelectEntity(entity.GetID());
		}

		if (opened) {
			if (hasChildren) {
				ImGui::Indent(10.0f);
				for (const auto& child : entity.GetTransform().GetChildren()) {
					DrawEntityNode(child->GetEntity());
				}
				ImGui::Unindent(10.0f);
				ImGui::TreePop();
			}
		}
	}

private:
	EditorContext& m_EditorContext;
};

}