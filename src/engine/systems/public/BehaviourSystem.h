#pragma once

#include "engine/utils/public/Singleton.h"
#include "engine/components/behaviours/public/Behaviour.h"
#include <vector>

namespace Engine::Systems {

class BehaviourSystem : public Utils::Singleton<BehaviourSystem> {
	friend class Singleton;

public:
	void Register(Components::Behaviours::Behaviour* behaviour);
	void Unregister(Components::Behaviours::Behaviour* behaviour);

	void Update();
	void FixedUpdate();
	void Clear();

private:
	BehaviourSystem() = default;

	void FlushPending();

	std::vector<Components::Behaviours::Behaviour*> m_Behaviours;
	std::vector<Components::Behaviours::Behaviour*> m_PendingAdd;
	std::vector<Components::Behaviours::Behaviour*> m_PendingRemove;
	bool m_IsIterating = false;
};

} // namespace Engine::Systems