#include <Leadwort/scenes/DefaultScene.h>
#include <Leadwort/systems/public/BehaviourSystem.h>
#include <Leadwort/systems/public/CameraSystem.h>
#include <Leadwort/systems/public/Input.h>
#include <Leadwort/systems/public/RenderSystem.h>
#include <Leadwort/systems/public/SceneSystem.h>

namespace Leadwort::Systems {

void SceneSystem::LoadScene(Unique<Core::IScene> scene) {
	m_PendingScene = std::move(scene);
	m_PendingSceneNeedsCreate = false;
}

void SceneSystem::LoadDefaultScene() {
	m_PendingScene = CreateUnique<Scenes::DefaultScene>();
	m_PendingSceneNeedsCreate = true;
}

void SceneSystem::LoadPendingScene() {
	if (m_PendingScene == nullptr) {
		return;
	}

	CameraSystem::Get().Clear();
	BehaviourSystem::Get().Clear();
	Input::Clear();

	m_CurrentScene = std::move(m_PendingScene);

	if (m_PendingSceneNeedsCreate) {
		m_CurrentScene->Create();
	}
	else {
		m_CurrentScene->InitComponents();
	}

	OnSceneLoaded.Execute(m_CurrentScene.get());
}

} // namespace Engine::Systems