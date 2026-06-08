#include "../public/Behaviour.h"
#include "engine/systems/public/BehaviourSystem.h"

namespace Engine::Components::Behaviours {

void Behaviour::SetEnabled(const bool value) {
	if (m_Enabled == value) {
		return;
	}

	m_Enabled = value;

	if (m_Enabled) {
		OnEnable();
	}
	else {
		OnDisable();
	}
}

void Behaviour::OnAdded() {
	Systems::BehaviourSystem::Get().Register(this);
	Start();

	if (m_Enabled) {
		OnEnable();
	}
}

void Behaviour::OnRemoved() {
	if (m_Enabled) {
		OnDisable();
	}

	OnDestroy();
	Systems::BehaviourSystem::Get().Unregister(this);
}

} // namespace Engine::Components