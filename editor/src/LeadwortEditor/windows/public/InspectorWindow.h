#pragma once
#include "Leadwort/core/math/public/Quat.h"
#include "Leadwort/core/math/public/Vec3.h"
#include "LeadwortEditor/core/public/FieldDrawerRegistry.h"
#include "LeadwortEditor/core/public/IEditorWindow.h"
#include "LeadwortEditor/data/EditorContext.h"
#include "imgui.h"

namespace Editor::Windows {

class InspectorWindow final : public Core::IEditorWindow {
public:
	explicit InspectorWindow(EditorContext& editorContext)
		: m_EditorContext(editorContext)
	{
		using namespace Leadwort::Components;

		Core::FieldDrawerRegistry::Get().Register(FieldType::Vec3, [](const FieldData& f) -> bool {
			auto* v = static_cast<Leadwort::Vec3*>(f.dataPtr);
			return ImGui::DragFloat3(f.displayName.c_str(), &v->x, 0.1f);
		});

		Core::FieldDrawerRegistry::Get().Register(FieldType::Quat, [](const FieldData& f) -> bool {
			auto* v = static_cast<Leadwort::Quat*>(f.dataPtr);
			Leadwort::Vec3 eulerAngles { v->ToEuler() };
			const bool changed = ImGui::DragFloat3(f.displayName.c_str(), &eulerAngles.x, 0.1f);
			if (changed) {
				*v = Leadwort::Quat::FromEuler(eulerAngles.x, eulerAngles.y, eulerAngles.z);
			}
			return changed;
		});

		Core::FieldDrawerRegistry::Get().Register(FieldType::Float, [](const FieldData& f) -> bool {
			return ImGui::DragFloat(f.displayName.c_str(), static_cast<float*>(f.dataPtr), 0.1f);
		});

		Core::FieldDrawerRegistry::Get().Register(FieldType::Bool, [](const FieldData& f) -> bool {
			return ImGui::Checkbox(f.displayName.c_str(), static_cast<bool*>(f.dataPtr));
		});

		Core::FieldDrawerRegistry::Get().Register(FieldType::Color, [](const FieldData& f) -> bool {
			return ImGui::ColorEdit4(f.displayName.c_str(), static_cast<float*>(f.dataPtr), ImGuiColorEditFlags_DisplayHSV);
		});
	}

	std::string_view GetName() const noexcept override { return "Inspector"; }
	
	void OnGuiRender() override {
		if (ImGui::Begin("Inspector")) {
			if (const auto* entityID = std::get_if<Leadwort::EntityID>(&m_EditorContext.selection)) {
				DrawEntityInspector(*entityID);
			}
			else if (const auto* asset = std::get_if<AssetSelection>(&m_EditorContext.selection)) {
				DrawAssetInspector(*asset);
			}
			else {
				ImGui::TextDisabled("Nothing selected");
			}
		}
		ImGui::End();
	}

private:
	void DrawEntityInspector(const Leadwort::EntityID entityID) const {
        if (entityID == Leadwort::Core::Entity::ROOT_ENTITY_ID) {
            return;
        }

        const auto* entity = m_EditorContext.openedScene->GetEntity(entityID);
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

                bool anyFieldChanged = false;
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

	static void DrawAssetInspector(const AssetSelection& asset) {
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
            default:
                ImGui::TextDisabled("No preview available for this asset type");
                break;
        }
    }

	static void DrawTexturePreview(const std::filesystem::path& path) {
        // Necesita GameAssets/EngineAssets según de qué raíz vino, o pasar
        // esa info también en AssetSelection si querés evitar ambigüedad
        // entre "texture.png" existiendo en ambas raíces.
		const auto texture = Leadwort::AssetManagement::GameAssets::GetTexture(path.string());

        if (texture) {
			const ImTextureID texID = static_cast<ImTextureID>(static_cast<intptr_t>(texture->GetGpuID()));
            ImGui::Image(texID, ImVec2(200, 200));
            ImGui::Text("Size: %dx%d", texture->GetWidth(), texture->GetHeight());
        }
        else {
            ImGui::TextDisabled("Failed to load texture");
        }
    }

	static void DrawModelPreview(const std::filesystem::path& path) {
		const auto model = Leadwort::AssetManagement::EngineAssets::GetModel(path.string());

        if (model) {
            ImGui::Text("Meshes: %zu", model->GetMeshCount());
        }
        else {
            ImGui::TextDisabled("Failed to load model");
        }
    }
	
private:
	EditorContext& m_EditorContext;
};

}