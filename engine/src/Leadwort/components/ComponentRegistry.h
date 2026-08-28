#pragma once
#include "Component.h"
#include "Leadwort/core/public/Entity.h"
#include "Leadwort/utils/public/Logger.h"

#include <algorithm>
#include <functional>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

namespace Leadwort::Components {

	class ComponentRegistry {
	public:
		using Factory = std::function<Component*(Core::Entity&)>;
		using AddPredicate = std::function<bool(const Core::Entity&)>;

		static ComponentRegistry& Get() {
			static ComponentRegistry instance;
			return instance;
		}

		template <IsComponent T>
		void Register(const std::string& typeName) {
			m_Types[typeName] = TypeInfo {
				.Create = [](Core::Entity& entity) -> Component* { return entity.AddComponent<T>(); },
				.CanAdd = [](const Core::Entity& entity) { return entity.CanAddComponent<T>(); }
			};
		}

		Component* CreateComponent(const std::string& typeName, Core::Entity& entity) {
			const auto it = m_Types.find(typeName);

			if (it == m_Types.end()) {
				LW_ERROR("ComponentRegistry: Unknown component type '", typeName, "', skipping. "
						  "This usually means a component class is missing LW_REFLECT(...).");
				return nullptr;
			}

			return it->second.Create(entity);
		}

		// The inspector builds its Add Component menu from this, so the order has to be
		// stable across frames: an unordered_map's iteration order is not.
		[[nodiscard]] std::vector<std::string> GetRegisteredTypeNames() const {
			std::vector<std::string> names{};
			names.reserve(m_Types.size());

			for (const auto& typeName : m_Types | std::views::keys) {
				names.push_back(typeName);
			}

			std::ranges::sort(names);

			return names;
		}

		[[nodiscard]] bool CanAddComponent(const std::string& typeName, const Core::Entity& entity) const {
			const auto it = m_Types.find(typeName);
			return it != m_Types.end() && it->second.CanAdd(entity);
		}

	private:
		struct TypeInfo {
			Factory Create{};
			AddPredicate CanAdd{};
		};

		std::unordered_map<std::string, TypeInfo> m_Types{};
	};

	#define LW_REGISTER_COMPONENT(Type) \
		namespace { \
			const bool _reg_##Type = [] {                                                                                      \
			ComponentRegistry::Get().Register<Type>(#Type); \
			return true; \
		}(); \
	}

}