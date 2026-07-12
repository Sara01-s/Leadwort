#pragma once

#include <Leadwort/utils/public/Singleton.h>

namespace Leadwort::Components::Behaviours {
	class DirectionalLight;
}

namespace Leadwort::Systems {

class LightingSystem : public Utils::Singleton<LightingSystem> {
	friend class Singleton;

public:
	void Register(Components::Behaviours::DirectionalLight* light);
	void Unregister();

	Components::Behaviours::DirectionalLight* GetDirectionalLight() const;
	bool IsEnabled() const;

private:
	LightingSystem() = default;

	Components::Behaviours::DirectionalLight* m_DirectionalLight = nullptr;
};

} // namespace Engine::Systems