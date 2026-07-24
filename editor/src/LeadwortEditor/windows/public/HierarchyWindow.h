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
	      const bool isChildOfRoot { hasParent && entity->GetTransform().GetParent()->GetEntity() == *m_EditorContext.openedScene->GetRootEntity() };

	      if (!hasParent || isChildOfRoot) {
	         DrawEntityNode(*entity, selectionChangedThisFrame);
	      }
	   }

	   if (selectionChangedThisFrame) {
	      ImGui::OpenPopup("Entity Selected##Hierarchy");
	   }

	   const ImVec2 center { ImGui::GetMainViewport()->GetCenter() };
	   ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	   if (ImGui::BeginPopup("Entity Selected##Hierarchy")) {
	      const std::string_view name { m_EditorContext.openedScene->GetEntity(m_EditorContext.selectedEntityID)->name };
	      ImGui::Text("Seleccionaste la entidad: %s", name.data());
	      if (ImGui::Button("OK")) {
	         ImGui::CloseCurrentPopup();
	      }
	      ImGui::EndPopup();
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