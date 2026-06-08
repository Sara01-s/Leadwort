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

	void LoadScene(std::unique_ptr<Core::Scene> scene);
	void LoadEmptyScene();
	void LoadPendingScene();

private:
	SceneSystem() = default;
	~SceneSystem() = default;

	std::unique_ptr<Core::Scene> m_CurrentScene = nullptr;
	std::unique_ptr<Core::Scene> m_PendingScene = nullptr;
};

} // namespace Engine::Systems