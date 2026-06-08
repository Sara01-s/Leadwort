
#include "engine/systems/public/CameraSystem.h"

#include "engine/systems/public/SceneSystem.h"

#include <ranges>
namespace Engine::Systems {

Components::Camera* CameraSystem::GetMain() {
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

	for (const auto& entity : scene->GetEntities() | std::ranges::views::values) {
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
