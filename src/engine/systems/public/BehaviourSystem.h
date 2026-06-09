#pragma once

#include "engine/utils/public/Singleton.h"
#include <vector>

namespace Engine::Components::Behaviours {
class Behaviour;
}

namespace Engine::Systems {

class BehaviourSystem : public Utils::Singleton<BehaviourSystem> {
	friend class Singleton;

public:
	void Register(Components::Behaviours::Behaviour* behaviour);
	void Unregister(Components::Behaviours::Behaviour* behaviour);

	void Enable(Components::Behaviours::Behaviour* behaviour);
	void Disable(Components::Behaviours::Behaviour* behaviour);

	void Update();
	void FixedUpdate();
	void Clear();

private:
	BehaviourSystem() = default;

	void FlushPending();

	std::vector<Components::Behaviours::Behaviour*> m_Behaviours;
	std::vector<Components::Behaviours::Behaviour*> m_ActiveBehaviours;

	std::vector<Components::Behaviours::Behaviour*> m_PendingAdd;
	std::vector<Components::Behaviours::Behaviour*> m_PendingRemove;
	std::vector<Components::Behaviours::Behaviour*> m_PendingActivate;
	std::vector<Components::Behaviours::Behaviour*> m_PendingDeactivate;

	bool m_IsIterating = false;
};

} // namespace Engine::Systems