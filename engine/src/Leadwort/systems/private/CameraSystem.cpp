
#include <Leadwort/systems/public/CameraSystem.h>
#include <Leadwort/systems/public/SceneSystem.h>

#include <ranges>
namespace Leadwort::Systems {

Components::Camera* CameraSystem::GetMainCamera() {
	if (m_Main == nullptr) {
		m_Main = FindByTag(Core::Tags::MAIN_CAMERA);
	}

	return m_Main;
}

Components::Camera* CameraSystem::GetSceneCamera() {
	if (m_SceneCamera == nullptr) {
		m_SceneCamera = FindByTag(Core::Tags::SCENE_CAMERA);
	}
	
	return m_SceneCamera;
}

Components::Camera* CameraSystem::FindByTag(const std::string& tag) {
	const auto* scene = SceneSystem::Get().GetCurrentScene();
	if (scene == nullptr) {
		return nullptr;
	}

	for (const auto& entity : scene->GetEntityMap() | std::ranges::views::values) {
		if (entity->CompareTag(tag) && entity->HasComponent<Components::Camera>()) {
			return entity->GetComponent<Components::Camera>();
		}
	}

	return nullptr;
}

void CameraSystem::Clear() {
	m_Main        = nullptr;
	m_SceneCamera = nullptr;
}

} // namespace Engine::Systems
