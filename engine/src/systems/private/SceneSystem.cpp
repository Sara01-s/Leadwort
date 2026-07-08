#include "systems/public/SceneSystem.h"

#include "scenes/EmptyScene.h"
#include "systems/public/BehaviourSystem.h"
#include "systems/public/CameraSystem.h"
#include "systems/public/Input.h"
#include "systems/public/RenderSystem.h"

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

	CameraSystem::Get().Clear();
	BehaviourSystem::Get().Clear();
	Input::Clear();

	m_CurrentScene = std::move(m_PendingScene);
	m_CurrentScene->Create();

	OnSceneLoaded.Execute(m_CurrentScene.get());
}

} // namespace Engine::Systems