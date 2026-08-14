#pragma once
#include "Component.h"
#include "Leadwort/core/public/Entity.h"
#include "Leadwort/utils/public/Logger.h"

#include <functional>
#include <string>
#include <unordered_map>

namespace Leadwort::Components {

	class ComponentRegistry {
	public:
		using Factory = std::function<Component*(Core::Entity&)>;

		static ComponentRegistry& Get() {
			static ComponentRegistry instance;
			return instance;
		}

		template <IsComponent T>
		void Register(const std::string& typeName) {
			m_Factories[typeName] = [](Core::Entity& entity) -> Component* {
				return entity.AddComponent<T>();
			};
		}

		Component* CreateComponent(const std::string& typeName, Core::Entity& entity) {
			const auto it = m_Factories.find(typeName);
			LW_ASSERT(it != m_Factories.end(), "Unknown component type: " + typeName);

			return it->second(entity);
		}

	private:
		std::unordered_map<std::string, Factory> m_Factories{};
	};

	// Macro para reducir boilerplate de registro:
	#define LW_REGISTER_COMPONENT(Type) \
		namespace { \
			const bool _reg_##Type = [] {                                                                                      \
			ComponentRegistry::Get().Register<Type>(#Type); \
			return true; \
		}(); \
	}

}