
#include "engine/systems/public/LightingSystem.h"

#include "engine/components/behaviours/public/DirectionaLight.h"
#include "engine/utils/public/Logger.h"

namespace Engine::Systems {

void LightingSystem::Register(Components::Behaviours::DirectionalLight* light) {
	CORE_LOG("[LightingSystem] Light registered: ", light->entity->name);
	m_DirectionalLight = light;
}

void LightingSystem::Unregister() {
	m_DirectionalLight = nullptr;
}

Components::Behaviours::DirectionalLight* LightingSystem::GetDirectionalLight() const {
	return m_DirectionalLight;
}

bool LightingSystem::IsEnabled() const {
	return m_DirectionalLight != nullptr;
}

} // namespace Engine::Systems