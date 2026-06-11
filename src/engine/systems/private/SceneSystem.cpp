#include "engine/systems/public/SceneSystem.h"

#include "engine/scenes/EmptyScene.h"
#include "engine/systems/public/BehaviourSystem.h"
#include "engine/systems/public/CameraSystem.h"
#include "engine/systems/public/Input.h"
#include "engine/systems/public/RenderSystem.h"

namespace Engine::Systems {

void SceneSystem::LoadScene(Unique<Core::Scene> scene) {
	m_PendingScene = std::move(scene);
}

void SceneSystem::LoadEmptyScene() {
	LoadScene(CreateUnique<Scenes::EmptyScene>());
}

void SceneSystem::LoadPendingScene() {
	if (m_PendingScene == nullptr) {
		return;
	}

	RenderSystem::Get().Clear();
	CameraSystem::Get().Clear();
	BehaviourSystem::Get().Clear();
	Input::Clear();

	m_CurrentScene = std::move(m_PendingScene);
	m_PendingScene = nullptr;

	m_CurrentScene->Create();

	OnSceneLoaded.Execute(m_CurrentScene.get());
}

} // namespace Engine::Systems