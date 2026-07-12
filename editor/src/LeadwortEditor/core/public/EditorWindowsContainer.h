#pragma once
#include "IEditorWindow.h"
#include "core/public/Core.h"

#include <vector>

namespace Editor::Core {

class EditorWindowsContainer {
public:
	EditorWindowsContainer() = default;
	~EditorWindowsContainer();

	template <class... Windows>
	void AddWindows(Windows&&... windows) {
		static_assert((std::is_convertible_v<Windows, Engine::Unique<IEditorWindow>> && ...));
		(m_Windows.emplace_back(std::forward<Windows>(windows)), ...);
	}

	void RenderAllWindows() const;

private:
	std::vector<Engine::Unique<IEditorWindow>> m_Windows{};
};

}