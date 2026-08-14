#pragma once
#include "LeadwortEditor/core/public/FieldDrawerRegistry.h"
#include "LeadwortEditor/core/public/IEditorWindow.h"
#include "LeadwortEditor/data/EditorContext.h"
#include "imgui.h"
#include "inspectors/MaterialInspector.h"

namespace Editor::Windows {

	class InspectorWindow final : public Core::IEditorWindow {
	public:
		explicit InspectorWindow(EditorContext& editorContext)
			: m_EditorContext(editorContext)
		{
			Core::FieldDrawerRegistry::RegisterBuiltInDrawers();
		}

		std::string_view GetName() const noexcept override { return "Inspector"; }

		void OnGuiRender() override {
			if (ImGui::Begin("Inspector")) {
				if (const auto* entityID = std::get_if<Leadwort::EntityID>(&m_EditorContext.Selection)) {
					DrawEntityInspector(*entityID);
				}
				else if (const auto* asset = std::get_if<AssetSelection>(&m_EditorContext.Selection)) {
					DrawAssetInspector(*asset);
				}
				else if (const auto* embeddedMesh = std::get_if<EmbeddedMeshSelection>(&m_EditorContext.Selection)) {
					if (embeddedMesh->mesh) {
						ImGui::Text("Mesh: %s", embeddedMesh->mesh->GetName().c_str());
						ImGui::TextDisabled("Embedded in model");
					}
				}
				else if (const auto* embeddedMat = std::get_if<EmbeddedMaterialSelection>(&m_EditorContext.Selection)) {
					if (embeddedMat->material) {
						ImGui::Text("%s", embeddedMat->material->GetName().c_str());
						ImGui::TextDisabled("Embedded material");
						DrawMaterialInspector(*embeddedMat->material);
					}
				}
				else {
					ImGui::TextDisabled("Nothing selected");
					m_CachedModelPath.clear();
				}
			}
			ImGui::End();
		}

	private:
		void DrawEntityInspector(const Leadwort::EntityID entityID) const {
	        if (entityID == Leadwort::Core::Entity::ROOT_ENTITY_ID) {
	            return;
	        }

	        const auto* entity { m_EditorContext.OpenedScene->GetEntity(entityID) };
	        if (!entity) {
	            ImGui::TextDisabled("No entity selected");
	            return;
	        }

	        ImGui::Text("%s", entity->name.c_str());
	        ImGui::Separator();

	        for (auto* component : entity->GetAllComponents()) {
	            ImGui::PushID(component);

	            if (ImGui::CollapsingHeader(component->GetTypeName().data(), ImGuiTreeNodeFlags_DefaultOpen)) {
	                if (auto* behaviour = dynamic_cast<Leadwort::Components::Behaviours::Behaviour*>(component)) {
	                    bool enabled = behaviour->IsEnabled();
	                    ImGui::Checkbox("Enabled", &enabled);
	                    behaviour->SetEnabled(enabled);
	                }

	                bool anyFieldChanged { false };
	                for (auto& field : component->GetFields()) {
	                    anyFieldChanged |= Core::FieldDrawerRegistry::Get().Draw(field);
	                }

	                if (anyFieldChanged) {
	                    component->OnFieldsChanged();
	                }
	            }

	            ImGui::PopID();
	        }
	    }

		void DrawAssetInspector(const AssetSelection& asset) {
			ImGui::Text("%s", asset.path.filename().string().c_str());
			ImGui::TextDisabled("%s", asset.path.string().c_str());
			ImGui::Separator();

			switch (asset.type) {
				case Leadwort::AssetManagement::AssetType::Texture:
					DrawTexturePreview(asset.path);
					break;
				case Leadwort::AssetManagement::AssetType::Model:
					DrawModelPreview(asset.path);
					break;
				case Leadwort::AssetManagement::AssetType::Material: {
					const auto material = Leadwort::AssetManagement::EngineAssets::GetMaterial(asset.path.string());
					if (material) {
						DrawMaterialInspector(*material);
					}
					else {
						ImGui::TextDisabled("Failed to load material");
					}
					break;
				}
				default:
					ImGui::TextDisabled("No preview is available for this asset type");
					break;
			}
		}

		static void DrawTexturePreview(const std::filesystem::path& path) {
			const auto texture = Leadwort::AssetManagement::EngineAssets::GetTexture(path.string());

	        if (texture) {
				const ImTextureID texID { static_cast<ImTextureID>(static_cast<intptr_t>(texture->GetGpuID())) };

	            ImGui::Image(texID, ImVec2(200, 200));
	            ImGui::Text("Size: %dx%d", texture->GetWidth(), texture->GetHeight());
	        }
	        else {
	            ImGui::TextDisabled("Failed to load texture");
	        }
	    }

		void DrawModelPreview(const std::filesystem::path& path) {
			if (m_CachedModelPath != path) {
				m_CachedModel = Leadwort::AssetManagement::EngineAssets::GetModel(path.string());
				m_CachedModelPath = path;
			}

			if (m_CachedModel) {
				ImGui::Text("Meshes: %zu", m_CachedModel->GetMeshCount());
			}
			else {
				ImGui::TextDisabled("Failed to load model");
			}
		}

	private:
		EditorContext& m_EditorContext;
		std::filesystem::path m_CachedModelPath{};
		Leadwort::Shared<Leadwort::Core::Model> m_CachedModel{};
	};

}