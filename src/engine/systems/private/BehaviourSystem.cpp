#include "../public/BehaviourSystem.h"

#include "engine/utils/public/Logger.h"

#include <engine/components/behaviours/public/Behaviour.h>

namespace Engine::Systems {

void BehaviourSystem::Register(Components::Behaviours::Behaviour* behaviour) {
	if (m_IsIterating) {
		m_PendingAdd.push_back(behaviour);
	}
	else {
		m_Behaviours.push_back(behaviour);
		m_ActiveBehaviours.push_back(behaviour);
		behaviour->Start();
		behaviour->OnEnable();
	}
}

void BehaviourSystem::Unregister(Components::Behaviours::Behaviour* behaviour) {
    if (m_IsIterating) {
        m_PendingRemove.push_back(behaviour);
    }
    else {
        behaviour->OnDisable();
        std::erase(m_Behaviours, behaviour);
        std::erase(m_ActiveBehaviours, behaviour);
    }
}

void BehaviourSystem::Enable(Components::Behaviours::Behaviour* behaviour) {
    if (m_IsIterating) {
        m_PendingActivate.push_back(behaviour);
    }
    else {
        m_ActiveBehaviours.push_back(behaviour);
        behaviour->OnEnable();
    }
}

void BehaviourSystem::Disable(Components::Behaviours::Behaviour* behaviour) {
    if (m_IsIterating) {
        m_PendingDeactivate.push_back(behaviour);
    }
    else {
        behaviour->OnDisable();
        std::erase(m_ActiveBehaviours, behaviour);
    }
}

void BehaviourSystem::Update() {
    FlushPending();
    m_IsIterating = true;

    for (auto* behaviour : m_ActiveBehaviours) {
        behaviour->Update();
    }

    m_IsIterating = false;
    FlushPending();
}

void BehaviourSystem::FixedUpdate() {
    FlushPending();
    m_IsIterating = true;

    for (auto* behaviour : m_ActiveBehaviours) {
        behaviour->FixedUpdate();
    }

    m_IsIterating = false;
    FlushPending();
}

void BehaviourSystem::Clear() {
    for (auto* behaviour : m_Behaviours) {
        behaviour->OnDisable();
    }

    m_Behaviours.clear();
    m_ActiveBehaviours.clear();
    m_PendingAdd.clear();
    m_PendingRemove.clear();
    m_PendingActivate.clear();
    m_PendingDeactivate.clear();
}

void BehaviourSystem::FlushPending() {
	for (auto* behaviour : m_PendingAdd) {
		m_Behaviours.push_back(behaviour);
		m_ActiveBehaviours.push_back(behaviour);
		behaviour->Start();
		behaviour->OnEnable();
	}
    m_PendingAdd.clear();

    for (auto* behaviour : m_PendingActivate) {
        m_ActiveBehaviours.push_back(behaviour);
        behaviour->OnEnable();
    }
    m_PendingActivate.clear();

    for (auto* behaviour : m_PendingDeactivate) {
        behaviour->OnDisable();
        std::erase(m_ActiveBehaviours, behaviour);
    }
    m_PendingDeactivate.clear();

    for (auto* behaviour : m_PendingRemove) {
        behaviour->OnDisable();
        std::erase(m_Behaviours, behaviour);
        std::erase(m_ActiveBehaviours, behaviour);
    }
    m_PendingRemove.clear();
}

} // namespace Engine::Systems