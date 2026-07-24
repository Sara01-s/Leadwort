#pragma once
#include "Leadwort/core/public/Scene.h"
#include "LeadwortEditor/core/public/IEditorWindow.h"

namespace Editor {

struct EditorContext final {
	Leadwort::EntityID selectedEntityID{};
	Leadwort::Core::Scene* openedScene{};

	std::vector<std::string> logHistory{};
	Leadwort::Utils::Event<const std::string&> logCallback{};
};

}