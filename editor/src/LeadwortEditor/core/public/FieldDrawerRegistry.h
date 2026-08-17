#pragma once
#include "Leadwort/components/Component.h"
#include "Leadwort/utils/public/PrimitiveMeshes.h"
#include "imgui.h"

#include <functional>

namespace Editor::Core {

using DrawerFn = std::function<bool(Leadwort::Components::FieldData&)>;

	class FieldDrawerRegistry {
	public:
		static FieldDrawerRegistry& Get() {
			static FieldDrawerRegistry instance{};
			return instance;
		}

		void Register(const Leadwort::Components::FieldType type, DrawerFn fn) {
			m_Drawers[type] = std::move(fn);
		}

		bool Draw(Leadwort::Components::FieldData& field) {
			if (const auto it { m_Drawers.find(field.Type) }; it != m_Drawers.end()) {
				return it->second(field);
			}

			ImGui::TextDisabled("%s (no drawer)", field.Name.c_str());
			return false;
		}

		static Leadwort::Vec3 SanitizeZeros(Leadwort::Vec3 v) {
			if (v.x == 0.0f) v.x = 0.0f;
			if (v.y == 0.0f) v.y = 0.0f;
			if (v.z == 0.0f) v.z = 0.0f;
			return v;
		}

		static void RegisterBuiltInDrawers() {
			using namespace Leadwort::Components;

			Get().Register(FieldType::Vec3, [](const FieldData& f) -> bool {
				auto* v = static_cast<Leadwort::Vec3*>(f.DataPtr);
				return ImGui::DragFloat3(f.DisplayName.c_str(), &v->x, 0.1f);
			});

			Get().Register(FieldType::Quat, [](const FieldData& f) -> bool {
				auto* v = static_cast<Leadwort::Quat*>(f.DataPtr);
				static std::unordered_map<void*, Leadwort::Vec3> s_EulerCache{};

				auto [it, inserted] { s_EulerCache.try_emplace(f.DataPtr) };
				Leadwort::Vec3& cachedEuler = it->second;

				if (inserted) {
					cachedEuler = SanitizeZeros(v->ToEuler());
				}

				const bool changed = ImGui::DragFloat3(f.DisplayName.c_str(), &cachedEuler.x, 0.1f);

				if (changed) {
					*v = Leadwort::Quat::FromEuler(cachedEuler.x, cachedEuler.y, cachedEuler.z);
				}
				else {
					const Leadwort::Quat reconstructed { Leadwort::Quat::FromEuler(cachedEuler.x, cachedEuler.y, cachedEuler.z) };
					if (!Leadwort::Quat::ApproximatelyEqual(reconstructed, *v)) {
						cachedEuler = v->ToEuler();
					}
				}

				return changed;
			});

			Get().Register(FieldType::Float, [](const FieldData& f) -> bool {
				return ImGui::DragFloat(f.DisplayName.c_str(), static_cast<float*>(f.DataPtr), 0.1f);
			});

			Get().Register(FieldType::Bool, [](const FieldData& f) -> bool {
				return ImGui::Checkbox(f.DisplayName.c_str(), static_cast<bool*>(f.DataPtr));
			});

			Get().Register(FieldType::Color, [](const FieldData& f) -> bool {
				return ImGui::ColorEdit4(f.DisplayName.c_str(), static_cast<float*>(f.DataPtr), ImGuiColorEditFlags_DisplayHSV);
			});

			Get().Register(FieldType::AssetRef, [](const FieldData& f) -> bool {

				auto* meshRef { static_cast<Leadwort::Shared<Leadwort::Rendering::Bindables::Mesh>*>(f.DataPtr) };

				const std::string currentLabel { *meshRef ? (*meshRef)->GetName() : "(None)" };

				bool changed{false};
				if (ImGui::BeginCombo(f.DisplayName.empty() ? "##AssetRef" : f.DisplayName.c_str(), currentLabel.c_str())) {
					if (ImGui::Selectable("(None)", !*meshRef)) {
						*meshRef = Leadwort::Utils::PrimitiveMeshes::Get().Empty();
						changed = true;
					}

					auto const& meshes { Leadwort::AssetManagement::EngineAssets::GetAllMeshes() };

					for (size_t i = 0; i < meshes.size(); ++i) {
						const auto& mesh = meshes[i];
						const std::string label = mesh->GetName().empty()
							? ("(Unnamed Mesh)##" + std::to_string(i))
							: (mesh->GetName() + "##" + std::to_string(i));

						const bool isSelected = (*meshRef == mesh);

						if (ImGui::Selectable(label.c_str(), isSelected)) {
							*meshRef = mesh;
							changed = true;
						}

						if (isSelected) {
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndCombo();
				}

				return changed;
			});

			Get().Register(FieldType::Enum, [](const FieldData& f) -> bool {
			   if (f.EnumNames.empty() || !f.DataPtr) return false;

			   auto* enumPtr = static_cast<int*>(f.DataPtr);
			   int currentItem = *enumPtr;

			   const int maxItems = static_cast<int>(f.EnumNames.size());
			   if (currentItem < 0 || currentItem >= maxItems) {
				  currentItem = 0;
			   }

			   if (ImGui::Combo(f.DisplayName.c_str(), &currentItem, f.EnumNames.data(), maxItems)) {
				  *enumPtr = currentItem;
				  return true;
			   }

			   return false;
			});
		}

	private:
		std::unordered_map<Leadwort::Components::FieldType, DrawerFn> m_Drawers;
	};

}