#pragma once
#include <string>

namespace Engine::Editor {

class EditorWindow {
public:
	virtual ~EditorWindow() = default;

	virtual std::string GetName() = 0;
	virtual void OnGuiRender() = 0;
};

}

