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

	std::string_view GetName() override {
		return "Hierarchy";
	}

	void OnGuiRender() override {
	   ImGui::Begin(GetName().data());
	   LW_ASSERT(m_EditorContext.openedScene != nullptr, "No scene opened");

	   bool selectionChangedThisFrame { false };

	   for (const auto& entity : m_EditorContext.openedScene->GetEntityMap() | std::views::values) {
	      const bool hasParent { entity->GetTransform().HasParent() };
	      const bool isChildOfRoot { hasParent && entity->GetTransform().GetParent()->GetEntity().GetID() == Leadwort::Core::Entity::ROOT_ENTITY_ID };

	      if (!hasParent || isChildOfRoot) {
	         DrawEntityNode(*entity, selectionChangedThisFrame);
	      }
	   }

	   ImGui::End();
	}

private:
	void DrawEntityNode(const Leadwort::Core::Entity& entity, bool& selectionChangedThisFrame) {
	   ImGuiTreeNodeFlags flags {
	      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth
	   };

	   const bool hasChildren { entity.GetTransform().HasChildren() };
	   if (!hasChildren) {
	      flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	   }

	   const bool opened { ImGui::TreeNodeEx(
	      reinterpret_cast<void*>(static_cast<intptr_t>(entity.GetID())),
	      flags,
	      "%s", entity.name.data()
	   ) };

	   if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
	      m_EditorContext.selectedEntityID = entity.GetID();
	      selectionChangedThisFrame = true;
	   }

	   if (opened) {
	      if (hasChildren) {
	         ImGui::Indent(10.0f);
	         for (const auto& child : entity.GetTransform().GetChildren()) {
	            DrawEntityNode(child->GetEntity(), selectionChangedThisFrame);
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