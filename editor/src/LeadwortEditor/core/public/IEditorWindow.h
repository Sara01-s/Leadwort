#pragma once
#include <string>

namespace Editor::Core {

class IEditorWindow {
	friend class EditorWindowsContainer;

public:
	virtual ~IEditorWindow() = default;

	constexpr virtual std::string_view GetName() = 0;
	virtual void OnGuiRender() = 0;
};

}

