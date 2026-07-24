#pragma once

#include <Leadwort/core/public/Scene.h>
#include <Leadwort/utils/public/Event.h>
#include <Leadwort/utils/public/Singleton.h>

namespace Leadwort::Systems {

class SceneSystem : public Utils::Singleton<SceneSystem> {
	friend class Singleton;

public:
	Utils::Event<const Core::Scene*> OnSceneLoaded;

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