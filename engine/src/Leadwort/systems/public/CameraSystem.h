#pragma once

#include <Leadwort/components/public/Camera.h>
#include <Leadwort/utils/public/Singleton.h>

#include <string>

namespace Leadwort::Systems {

class CameraSystem : public Utils::Singleton<CameraSystem> {
	friend class Singleton;

public:
	Components::Camera* GetMainCamera();
	Components::Camera* GetSceneCamera();

	void Clear();

private:
	CameraSystem() = default;
	~CameraSystem() { Clear(); }

	static Components::Camera* FindByTag(const std::string& tag);

	Components::Camera* m_Main        = nullptr;
	Components::Camera* m_SceneCamera = nullptr;
};

} // namespace Engine::Systems