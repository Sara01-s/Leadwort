#pragma once

#include "../../Component.h"
#include "engine/core/public/Entity.h"

namespace Engine::Components::Behaviours {

class Transform;

class Behaviour : public Component {
public:
	using Base = Component;

	[[nodiscard]] Components::Transform* GetTransform() const { return entity->transform; }

	void SetEnabled(bool value);
	[[nodiscard]] bool IsEnabled() const { return m_Enabled; }

	virtual void Start()       {}
	virtual void Update()      {}
	virtual void FixedUpdate() {}
	virtual void OnDestroy()   {}
	virtual void OnEnable()    {}
	virtual void OnDisable()   {}

	template <typename T>
	T* GetComponent() const { return entity->GetComponent<T>(); }

	template <typename T>
	bool HasComponent() const { return entity->HasComponent<T>(); }

	void OnAdded()   final;
	void OnRemoved() final;

private:
	bool m_Enabled = true;
};

} // namespace Engine::Components::Behaviours