#pragma once

#include "engine/utils/public/ReactiveCommand.h"
#include "engine/utils/public/Singleton.h"
#include <engine/core/public/Scene.h>

namespace Engine::Systems {

class SceneSystem : public Utils::Singleton<SceneSystem> {
	friend class Singleton;

public:
	Utils::ReactiveCommand<const Core::Scene*> OnSceneLoaded;

	[[nodiscard]] Core::Scene* GetCurrentScene() const { return m_CurrentScene.get(); }

	void LoadScene(Unique<Core::Scene> scene);
	void LoadEmptyScene();
	void LoadPendingScene();

private:
	SceneSystem() = default;
	~SceneSystem() = default;

	Unique<Core::Scene> m_CurrentScene = nullptr;
	Unique<Core::Scene> m_PendingScene = nullptr;
};

} // namespace Engine::Systems