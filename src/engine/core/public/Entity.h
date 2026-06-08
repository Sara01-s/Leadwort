#pragma once

#include "Layers.h"
#include "Tags.h"
#include "engine/components/public/Transform.h"

#include <engine/components/Component.h>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace Engine::Core { class Scene; }

namespace Engine::Core {

class Entity {
public:
    static constexpr auto DEFAULT_NAME = "New Entity";

    const int id;
    const std::string name;

    Components::Transform* transform = nullptr;

    uint32_t layerMask = Utils::Layers::EVERYTHING;
    std::string tag = Tags::DEFAULT;

	Scene* scene = nullptr;

	explicit Entity(int id, std::string name = DEFAULT_NAME);
    ~Entity();

    Entity(const Entity&)            = delete;
    Entity& operator=(const Entity&) = delete;

    // ── Component management ─────────────────────────────────────────────────

	template <Components::IsComponent T>
	T* AddComponent() {
		if (HasComponent<T>()) {
			throw std::runtime_error("Duplicate component: " + std::string(typeid(T).name()));
		}

		auto component = std::make_unique<T>();
		T* componentPtr = component.get();

		componentPtr->entity = this;
		componentPtr->OnAdded();
		RegisterParents<T>(componentPtr);

		m_Components[typeid(T)] = std::move(component);

		return componentPtr;
	}

    template <typename T>
    void RemoveComponent() {
        const std::type_index key = typeid(T);
		const auto it = m_Components.find(key);

        if (it == m_Components.end()) {
            throw std::runtime_error(std::string("Removing non-existent component: ") + typeid(T).name());
        }

        it->second->OnRemoved();
        UnregisterComponent<T>(it->second);
    }

	template <typename T>
	T* GetComponent() {
		const auto it = m_Components.find(std::type_index(typeid(T)));
		if (it == m_Components.end()) {
			return nullptr;
		}

		return static_cast<T*>(it->second.get());
	}

    template <typename T>
    [[nodiscard]] T* TryGetComponent() const {
		const auto it = m_Components.find(typeid(T));
        return it != m_Components.end() ? static_cast<T*>(it->second) : nullptr;
    }

    template <typename T>
    [[nodiscard]] bool HasComponent() const {
        return m_Components.contains(typeid(T));
    }

	[[nodiscard]] std::vector<Components::Component*> GetAllComponents() const {
		std::vector<Components::Component*> components;
		components.reserve(m_Components.size());

		for (const auto& comp: m_Components | std::views::values) {
			components.push_back(comp.get());
		}

		return components;
	}

    // ── Accessors ────────────────────────────────────────────────────────────

    [[nodiscard]] bool CompareTag(const std::string& t) const { return tag == t; }
    Entity* FindEntityByTag(const std::string& tag) const;
    Entity* CreateChild(const std::string& childName = DEFAULT_NAME) const;

private:
	template <Components::IsComponent T>
	void RegisterParents(Components::Component* component) {
		using Base = Components::BaseOf_t<T>;

		if constexpr (!std::is_same_v<Base, void> && !std::is_same_v<Base, Components::Component>) {
			m_Components[typeid(Base)] = component;
			RegisterParents<Base>(component);
		}
	}

    template <typename T>
    void UnregisterComponent(Components::Component* component) {
        m_Components.erase(typeid(T));
        UnregisterParents<T>(component);
        delete component;
    }

    template <typename T>
    void UnregisterParents(Components::Component* component) {
        using Base = T::Base;
        if constexpr (!std::is_same_v<Base, Components::Component>) {
            m_Components.erase(typeid(Base));
            UnregisterParents<Base>(component);
        }
    }

    std::unordered_map<std::type_index, std::unique_ptr<Components::Component>> m_Components;
};

} // namespace Engine::Core