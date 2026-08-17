#pragma once

#include <Leadwort/events/public/Event.h>
#include <Leadwort/core/public/IScene.h>
#include <Leadwort/utils/public/Singleton.h>

namespace Leadwort::Systems {

class SceneSystem : public Utils::Singleton<SceneSystem> {
	friend class Singleton;

public:
	Events::Event<const Core::IScene*> OnSceneLoaded{};

	[[nodiscard]] Core::IScene* GetCurrentScene() const { return m_CurrentScene.get(); }

	void LoadScene(Unique<Core::IScene> scene);
	void LoadDefaultScene();
	void LoadPendingScene();

private:
	SceneSystem() = default;
	~SceneSystem() = default;

	Unique<Core::IScene> m_CurrentScene = nullptr;
	Unique<Core::IScene> m_PendingScene = nullptr;
	bool m_PendingSceneNeedsCreate { false };
};

} // namespace Engine::Systems