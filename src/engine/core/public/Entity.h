#pragma once

#include "Core.h"
#include "Layers.h"
#include "Tags.h"
#include "engine/utils/public/Logger.h"

#include <engine/components/Component.h>
#include <memory>
#include <ranges>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Engine::Components {
	class Transform;
}

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

    template <typename T>
    T* AddComponent() {
        static_assert(std::is_base_of_v<Components::Component, T>, "T must derive from Component");

        const std::type_index key = typeid(T);

        CORE_ASSERT(!m_Components.contains(key), std::string("Entity '") + name + "' already has component: " + typeid(T).name());

        auto owner = CreateUnique<T>();
        T* ptr = owner.get();
        ptr->entity = this;

        m_OwnedComponents.push_back(std::move(owner));
        m_Components[key] = ptr;

        RegisterParents<T>(ptr);

        ptr->OnAdded();

        return ptr;
    }

    template <typename T>
    void RemoveComponent() {
        const std::type_index key = typeid(T);
        const auto it = m_Components.find(key);

        CORE_ASSERT(it != m_Components.end(), std::string("Entity '") + name + "' removing non-existent component: " + typeid(T).name());

        Components::Component* raw = it->second;
        raw->OnRemoved();

        UnregisterParents<T>(raw);
        m_Components.erase(key);

        const auto ownedIt = std::ranges::find_if(m_OwnedComponents, [raw](const auto& owned) {
            return owned.get() == raw;
        });

        CORE_ASSERT(ownedIt != m_OwnedComponents.end(), std::string("Entity '") + name + "' component not found in owned list: " + typeid(T).name());

        m_OwnedComponents.erase(ownedIt);
    }

    template <typename T>
    T* GetComponent() const {
        const auto it = m_Components.find(std::type_index(typeid(T)));
        if (it == m_Components.end()) { return nullptr; }
        return static_cast<T*>(it->second);
    }

    template <typename T>
    [[nodiscard]] bool HasComponent() const {
        return m_Components.contains(std::type_index(typeid(T)));
    }

    [[nodiscard]] std::vector<Components::Component*> GetAllComponents() const {
        std::vector<Components::Component*> result;
        result.reserve(m_OwnedComponents.size());

        for (const auto& owned : m_OwnedComponents) {
            result.push_back(owned.get());
        }

        return result;
    }

    [[nodiscard]] bool CompareTag(const std::string& t) const { return tag == t; }
    [[nodiscard]] Entity* FindEntityByTag(const std::string& tag) const;
    Entity* CreateChild(const std::string& childName) const;

    template <typename TFunc>
    Entity* CreateChild(const std::string& childName, TFunc&& init) const {
        Entity* child = CreateChild(childName);
        std::forward<TFunc>(init)(*child);
        return child;
    }

private:
    template <Components::IsComponent T>
    void RegisterParents(Components::Component* component) {
        using Base = Components::BaseOf_t<T>;

        if constexpr (!std::is_same_v<Base, void> && !std::is_same_v<Base, Components::Component>) {
            CORE_ASSERT(!m_Components.contains(typeid(Base)), std::string("Entity '") + name + "' base type already registered: " + typeid(Base).name());

            m_Components[typeid(Base)] = component;
            RegisterParents<Base>(component);
        }
    }

    template <typename T>
    void UnregisterParents(Components::Component* component) {
        using Base = T::Base;

        if constexpr (!std::is_same_v<Base, Components::Component>) {
            m_Components.erase(typeid(Base));
            UnregisterParents<Base>(component);
        }
    }

    std::unordered_map<std::type_index, Components::Component*> m_Components;
    std::vector<Unique<Components::Component>> m_OwnedComponents;
};

} // namespace Engine::Core

#include "engine/components/behaviours/private/Behaviour.inl"