#pragma once

#include "Core.h"
#include "Layers.h"
#include "Tags.h"
#include <Leadwort/utils/public/Logger.h>
#include <Leadwort/components/Component.h>

#include <memory>
#include <ranges>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Leadwort::Components {
	class Transform;
}

namespace Leadwort::Core { class Scene; }

namespace Leadwort::Core {

	class Entity final : public Serialization::ISerializable {
	public:
		static constexpr EntityID ROOT_ENTITY_ID { 0U };
	    static constexpr auto DEFAULT_NAME { "New Entity" };

	    std::string name { DEFAULT_NAME };
	    uint32_t layerMask { Utils::Layers::EVERYTHING };
	    std::string tag { Tags::DEFAULT };
		Scene* scene { nullptr };

	public:
	    explicit Entity(EntityID id, std::string name = DEFAULT_NAME);
	    ~Entity() override;

	    Entity(const Entity&) = delete;
	    Entity& operator=(const Entity&) = delete;
		constexpr bool operator==(const Entity& other) const noexcept { return m_ID == other.GetID(); }

		Components::Transform& GetTransform() const {
			return *m_Transform;
		}

	    template <Components::IsComponent TComponent>
	    TComponent* AddComponent() {
	        static_assert(std::is_base_of_v<Components::Component, TComponent>, "T must derive from Component");

	        const std::type_index key = typeid(TComponent);

	        LW_ASSERT(!m_Components.contains(key), std::string("Entity '") + name + "' already has component: " + typeid(TComponent).name());

	        auto owner = CreateUnique<TComponent>();
	        TComponent* ownerBorrowedPtr = owner.get();
	        ownerBorrowedPtr->SetEntity(*this);

	        m_OwnedComponents.push_back(std::move(owner));
	        m_Components[key] = ownerBorrowedPtr;

	        RegisterParents<TComponent>(ownerBorrowedPtr);

	        ownerBorrowedPtr->OnAdded();

	        return ownerBorrowedPtr;
	    }

	    template <Components::IsComponent TComponent>
	    void RemoveComponent() {
	        const std::type_index key = typeid(TComponent);
	        const auto it = m_Components.find(key);

	        LW_ASSERT(it != m_Components.end(), std::string("Entity '") + name + "' removing non-existent component: " + typeid(TComponent).name());

	        Components::Component* raw = it->second;
	        raw->OnRemoved();

	        UnregisterParents<TComponent>(raw);
	        m_Components.erase(key);

	        const auto ownedIt = std::ranges::find_if(m_OwnedComponents, [raw](const auto& owned) {
	            return owned.get() == raw;
	        });

	        LW_ASSERT(ownedIt != m_OwnedComponents.end(), std::string("Entity '") + name + "' component not found in owned list: " + typeid(TComponent).name());

	        m_OwnedComponents.erase(ownedIt);
	    }

	    template <Components::IsComponent TComponent>
	    TComponent* GetComponent() const {
	        const auto it = m_Components.find(std::type_index(typeid(TComponent)));
	        return it == m_Components.end() ? nullptr : static_cast<TComponent*>(it->second);
		}

	    template <Components::IsComponent TComponent>
	    [[nodiscard]] bool HasComponent() const {
	        return m_Components.contains(std::type_index(typeid(TComponent)));
	    }

	    [[nodiscard]] std::vector<Components::Component*> GetAllComponents() const {
	        std::vector<Components::Component*> result{};
	        result.reserve(m_OwnedComponents.size());

	        for (const auto& owned : m_OwnedComponents) {
	            result.push_back(owned.get());
	        }

	        return result;
	    }

	    [[nodiscard]] bool CompareTag(const std::string& t) const { return tag == t; }
		[[nodiscard]] constexpr EntityID GetID() const { return m_ID; }
	    [[nodiscard]] Entity* FindEntityByTag(const std::string& t) const;
	    Entity* CreateChild(const std::string& childName) const;

	    template <typename TFunc>
	    Entity* CreateChild(const std::string& childName, TFunc&& init) const {
	        Entity* child = CreateChild(childName);
	        std::forward<TFunc>(init)(*child);

	        return child;
	    }

	public:
		void Serialize(Json& out) const override;
		void Deserialize(const Json& in) override;
		std::string_view GetTypeName() const override { return "Entity"; }

	private:
	    template <Components::IsComponent TComponent>
	    void RegisterParents(Components::Component* component) {
	        using Base = BaseOf_t<TComponent>;

	        if constexpr (!std::is_same_v<Base, void> && !std::is_same_v<Base, Components::Component>) {
	            LW_ASSERT(!m_Components.contains(typeid(Base)), std::string("Entity '") + name + "' base type already registered: " + typeid(Base).name());

	            m_Components[typeid(Base)] = component;
	            RegisterParents<Base>(component);
	        }
	    }

	    template <Components::IsComponent TComponent>
	    void UnregisterParents(Components::Component* component) {
	        using Base = TComponent::Base;

	        if constexpr (!std::is_same_v<Base, Components::Component>) {
	            m_Components.erase(typeid(Base));
	            UnregisterParents<Base>(component);
	        }
	    }

	private:
	    std::unordered_map<std::type_index, Components::Component*> m_Components{};
	    std::vector<Unique<Components::Component>> m_OwnedComponents{};
		Components::Transform* m_Transform;
	    EntityID m_ID { ROOT_ENTITY_ID };
	};

	}

#include <Leadwort/components/behaviours/private/Behaviour.inl>