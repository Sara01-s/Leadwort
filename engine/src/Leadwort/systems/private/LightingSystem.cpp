#include <Leadwort/systems/public/LightingSystem.h>
#include <Leadwort/components/behaviours/public/DirectionaLight.h>
#include <Leadwort/core/public/Entity.h>
#include <Leadwort/utils/public/Logger.h>

namespace Leadwort::Systems {

void LightingSystem::Register(Components::Behaviours::DirectionalLight* light) {
	LW_LOG("LightingSystem: Light registered: ", light->GetEntity().name);
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