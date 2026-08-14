#pragma once

#include <filesystem>
#include <variant>
#include "imgui.h"
#include "ImGuizmo.h"

namespace Leadwort::AssetManagement {
	enum class AssetType;
}

namespace Leadwort::Rendering::Bindables {
	class Mesh;
	class Material;
}

namespace Editor {

	struct AssetSelection {
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
			AssetSelection,
			EmbeddedMeshSelection,
			EmbeddedMaterialSelection
		> Selection{};

		Leadwort::Core::Scene* OpenedScene{};
		std::vector<std::string> LogHistory{};
		Leadwort::Utils::Event<const std::string&> LogCallback{};

		ImGuizmo::OPERATION GizmoOperation { ImGuizmo::TRANSLATE };
		ImGuizmo::MODE GizmoMode { ImGuizmo::LOCAL };

	public:
		void SelectEntity(Leadwort::EntityID id) {
			Selection = id;
		}

		void SelectAsset(const std::filesystem::path& path, const Leadwort::AssetManagement::AssetType type) {
			Selection = AssetSelection { path, type };
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

		[[nodiscard]] bool HasSelection() const noexcept {
			return !std::holds_alternative<std::monostate>(Selection);
		}
	};

}