#pragma once
#include "engine/utils/public/Logger.h"

#include <concepts>

namespace Engine::Core { class Entity; }

namespace Engine::Components {

class Component {
public:
	virtual ~Component() = default;

	void SetEntity(Core::Entity& entity) {
		m_Entity = &entity;
	}

	[[nodiscard]] Core::Entity& GetEntity() const {
		CORE_ASSERT(m_Entity, "Invalid component: No entity assigned.");
		return *m_Entity;
	}

	virtual void OnAdded()   {}
	virtual void OnRemoved() {}

private:
	Core::Entity* m_Entity = nullptr;
};

template<typename T>
concept IsComponent = std::derived_from<T, Component> && std::is_default_constructible_v<T>;

template<typename T>
concept HasBaseClass = requires { typename T::Base; };

template <typename>
	struct BaseOf {
	using type = void;
};

template <typename T>
requires requires { typename T::Base; }
struct BaseOf<T> {
	using type = T::Base;
};

template <typename T>
	using BaseOf_t = BaseOf<T>::type;
} // namespace Engine::Components