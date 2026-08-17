#pragma once

#include "imgui.h"
#include "ImGuizmo.h"
#include "Leadwort/scenes/DefaultScene.h"
#include <filesystem>
#include <variant>

namespace Leadwort::AssetManagement {
	enum class AssetType;
}

namespace Leadwort::Rendering::Bindables {
	class Mesh;
	class Material;
}

namespace Editor {

	struct EditorSelection {
		std::filesystem::path path{};
		Leadwort::AssetManagement::AssetType type{};
	};

	struct EmbeddedMeshSelection {
		Leadwort::Shared<Leadwort::Rendering::Bindables::Mesh> mesh{};
	};

	struct EmbeddedMaterialSelection {
		Leadwort::Shared<Leadwort::Rendering::Bindables::Material> material{};
	};

	class EditorContext final {
	public:
		std::variant<
			std::monostate,
			Leadwort::EntityID,
			EditorSelection,
			EmbeddedMeshSelection,
			EmbeddedMaterialSelection
		> Selection{};

		Leadwort::Core::IScene* OpenedScene{};
		std::filesystem::path OpenedScenePath{};
		std::vector<std::string> LogHistory{};
		Leadwort::Events::Event<const std::string&> LogCallback{};

		ImGuizmo::OPERATION GizmoOperation { ImGuizmo::TRANSLATE };
		ImGuizmo::MODE GizmoMode { ImGuizmo::LOCAL };

	public:
		void SelectEntity(Leadwort::EntityID id) {
			Selection = id;
		}

		void SelectAsset(const std::filesystem::path& path, const Leadwort::AssetManagement::AssetType type) {
			Selection = EditorSelection { path, type };
		}

		void SelectEmbeddedMesh(const Leadwort::Shared<Leadwort::Rendering::Bindables::Mesh>& mesh) {
			Selection = EmbeddedMeshSelection { mesh };
		}

		void SelectEmbeddedMaterial(const Leadwort::Shared<Leadwort::Rendering::Bindables::Material>& material) {
			Selection = EmbeddedMaterialSelection { material };
		}

		void ClearSelection() {
			Selection = std::monostate{};
		}

		[[nodiscard]]
		bool HasSelection() const noexcept {
			return !std::holds_alternative<std::monostate>(Selection);
		}

		[[nodiscard]]
		bool HasOpenedScene() const noexcept {
			return OpenedScene != nullptr;
		}
	};

}