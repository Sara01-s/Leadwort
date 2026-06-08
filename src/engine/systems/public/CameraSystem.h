#pragma once

#include "engine/components/public/Camera.h"
#include "engine/utils/public/Singleton.h"

namespace Engine::Systems {

class CameraSystem : public Utils::Singleton<CameraSystem> {
	friend class Singleton;

public:
	Components::Camera* GetMain();
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