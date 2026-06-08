#include "../public/BehaviourSystem.h"
#include <engine/components/behaviours/public/Behaviour.h>

namespace Engine::Systems {

void BehaviourSystem::Register(Components::Behaviours::Behaviour* behaviour) {
	if (m_IsIterating) {
		m_PendingAdd.push_back(behaviour);
	}
	else {
		m_Behaviours.push_back(behaviour);
	}
}

void BehaviourSystem::Unregister(Components::Behaviours::Behaviour* behaviour) {
	if (m_IsIterating) {
		m_PendingRemove.push_back(behaviour);
	}
	else {
		std::erase(m_Behaviours, behaviour);
	}
}

void BehaviourSystem::Update() {
	FlushPending();
	m_IsIterating = true;

	for (auto* behaviour: m_Behaviours) {
		if (behaviour->IsEnabled()) {
			behaviour->Update();
		}
	}

	m_IsIterating = false;
	FlushPending();
}

void BehaviourSystem::FixedUpdate() {
	FlushPending();
	m_IsIterating = true;

	for (auto* behaviour: m_Behaviours) {
		if (behaviour->IsEnabled()) {
			behaviour->FixedUpdate();
		}
	}

	m_IsIterating = false;
	FlushPending();
}

void BehaviourSystem::Clear() {
	m_Behaviours.clear();
	m_PendingAdd.clear();
	m_PendingRemove.clear();
}

void BehaviourSystem::FlushPending() {
	if (!m_PendingAdd.empty()) {
		m_Behaviours.insert(m_Behaviours.end(), m_PendingAdd.begin(), m_PendingAdd.end());
		m_PendingAdd.clear();
	}

	if (!m_PendingRemove.empty()) {

		for (auto* behaviour: m_PendingRemove) {
			std::erase(m_Behaviours, behaviour);
		}

		m_PendingRemove.clear();
	}
}

} // namespace Engine::Systems