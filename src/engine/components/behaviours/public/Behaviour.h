#pragma once

#include "engine/components/Component.h"
#include "engine/systems/public/BehaviourSystem.h"

// Forward declarations — Entity is NOT yet complete here.
namespace Engine::Core       { class Entity; }
namespace Engine::Components { class Transform; }

namespace Engine::Components::Behaviours {

class Behaviour : public Component {
public:
	// Declarations only — definitions are in Behaviour.inl,
	// included at the bottom of Entity.h after Entity is complete.
	[[nodiscard]] Transform* GetTransform() const;

	template <typename T>
	T* GetComponent() const;

	template <typename T>
	bool HasComponent() const;

	virtual void OnEnable()  {}
	virtual void OnDisable() {}
	virtual void Start() {}
	virtual void Update() {}
	virtual void FixedUpdate() {}
	virtual void OnInspector() {}

	void OnAdded() final {
		Systems::BehaviourSystem::Get().Register(this);
	}

	void OnRemoved() final {
		Systems::BehaviourSystem::Get().Unregister(this);
	}

	bool IsEnabled() const { return m_IsEnabled; }

	void SetEnabled(const bool enabled) {
		if (m_IsEnabled == enabled) {
			return;
		}

		m_IsEnabled = enabled;
		m_IsEnabled ? OnEnable() : OnDisable();
	}

private:
	bool m_IsEnabled = true;
};

} // namespace Engine::Components::Behaviours