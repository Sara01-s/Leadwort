#include <Leadwort/systems/public/SceneSystem.h>
#include <Leadwort/scenes/EmptyScene.h>
#include <Leadwort/systems/public/BehaviourSystem.h>
#include <Leadwort/systems/public/CameraSystem.h>
#include <Leadwort/systems/public/Input.h>
#include <Leadwort/systems/public/RenderSystem.h>

namespace Leadwort::Systems {

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