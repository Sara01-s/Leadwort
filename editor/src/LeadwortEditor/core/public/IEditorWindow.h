#pragma once
#include <string>

namespace Editor::Core {

class IEditorWindow {
	friend class EditorWindowsContainer;

public:
	virtual ~IEditorWindow() = default;

	virtual std::string_view GetName() const noexcept = 0;
	virtual void OnGuiRender() = 0;
};

}

