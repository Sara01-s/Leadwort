#include "../public/EditorWindowsContainer.h"

#include <Leadwort/core/math/public/Vec2.h>

namespace Editor::Core {

	EditorWindowsContainer::~EditorWindowsContainer() {
		m_Windows.clear();
	}

	void EditorWindowsContainer::RenderAllWindows() const {
		for (auto const& window : m_Windows) {
			window->OnGuiRender();
		}
	}

} // namespace Editor::Core