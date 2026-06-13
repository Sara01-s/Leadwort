#pragma once

// This file is included at the bottom of Entity.h, where Entity is fully defined.
// Never include this file directly.

#include "engine/components/behaviours/public/Behaviour.h"
// Entity.h is already included by whoever includes this.

namespace Engine::Components::Behaviours {

inline Transform& Behaviour::GetTransform() const {
	return GetEntity().GetTransform();
}

template <typename T>
T* Behaviour::GetComponent() const {
	return GetEntity().GetComponent<T>();
}

template <typename T>
bool Behaviour::HasComponent() const {
	return GetEntity().HasComponent<T>();
}

} // namespace Engine::Components::Behaviours