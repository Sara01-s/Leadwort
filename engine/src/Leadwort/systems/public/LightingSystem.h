#pragma once

#include <Leadwort/utils/public/Singleton.h>
#include <Leadwort/rendering/public/LightingUBO.h>

namespace Leadwort::Components::Behaviours {
	class Light;
}

namespace Leadwort::Systems {

class LightingSystem : public Utils::Singleton<LightingSystem> {
	friend class Singleton;
public:
	using Lights = std::array<Components::Behaviours::Light*, Rendering::MAX_LIGHTS>;
	void Register(Components::Behaviours::Light* light);
	void Unregister(const Components::Behaviours::Light* light);

	[[nodiscard]] Lights GetLights() const noexcept { return m_Lights; }

private:
	LightingSystem() = default;
	Lights m_Lights{};
};

} // namespace Engine::Systems