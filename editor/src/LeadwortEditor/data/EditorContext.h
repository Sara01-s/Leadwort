#pragma once

#include <filesystem>
#include <variant>
#include "imgui.h"
#include "ImGuizmo.h"

namespace Leadwort::AssetManagement {
	enum class AssetType;
}

namespace Editor {

	struct AssetSelection {
		std::filesystem::path path{};
		Leadwort::AssetManagement::AssetType type{};
	};

	class EditorContext final {
	public:
		std::variant<std::monostate, Leadwort::EntityID, AssetSelection> selection{};
		Leadwort::Core::Scene* openedScene{};
		std::vector<std::string> logHistory{};
		Leadwort::Utils::Event<const std::string&> logCallback{};

		ImGuizmo::OPERATION gizmoOperation { ImGuizmo::TRANSLATE };
		ImGuizmo::MODE gizmoMode { ImGuizmo::LOCAL };

	public:
		void SelectEntity(Leadwort::EntityID id) {
			selection = id;
		}

		void SelectAsset(const std::filesystem::path& path, Leadwort::AssetManagement::AssetType type) {
			selection = AssetSelection { path, type };
		}

		void ClearSelection() {
			selection = std::monostate{};
		}
	};

}