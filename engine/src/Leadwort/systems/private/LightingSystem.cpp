#include <Leadwort/components/behaviours/public/Light.h>
#include <Leadwort/core/public/Entity.h>
#include <Leadwort/systems/public/LightingSystem.h>
#include <Leadwort/utils/public/Logger.h>

namespace Leadwort::Systems {

void LightingSystem::Register(Components::Behaviours::Light* light) {
	if (!light) {
		return;
	}

	for (auto& slot : m_Lights) {
		if (slot == light) {
			return;
		}

		if (slot == nullptr) {
			slot = light;
			LW_LOG("LightingSystem: Light registered: ", light->GetEntity().name, " Light Type: ", light->GetTypeAsString());
			return;
		}
	}

	LW_ASSERT(false, "Too many lights! Exceeded MAX_LIGHTS capacity.");
}

void LightingSystem::Unregister(const Components::Behaviours::Light* light) {
	if (!light) {
		return;
	}

	for (auto& slot : m_Lights) {
		if (slot == light) {
			slot = nullptr;
			LW_LOG("LightingSystem: Light unregistered: ", light->GetEntity().name);
			return;
		}
	}
}

} // namespace Engine::Systems