#pragma once
#include "Leadwort/core/public/Scene.h"
#include "LeadwortEditor/core/public/IEditorWindow.h"

namespace Editor {

struct EditorContext final {
	Leadwort::EntityID selectedEntityID{};
	Core::IEditorWindow* focusedWindow{};
	Leadwort::Core::Scene* openedScene{};
};

}