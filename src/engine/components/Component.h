#pragma once
#include <concepts>

namespace Engine::Core { class Entity; }

namespace Engine::Components {

class Component {
public:
	virtual ~Component() = default;

	virtual void OnAdded()   {}
	virtual void OnRemoved() {}

	Core::Entity* entity = nullptr;
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