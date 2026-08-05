#pragma once
#include "Leadwort/components/Component.h"
#include "imgui.h"

#include <functional>

namespace Editor::Core {

using DrawerFn = std::function<bool(Leadwort::Components::FieldData&)>;

class FieldDrawerRegistry {
public:
	static FieldDrawerRegistry& Get() {
		static FieldDrawerRegistry instance;
		return instance;
	}

	void Register(const Leadwort::Components::FieldType type, DrawerFn fn) {
		m_Drawers[type] = std::move(fn);
	}

	bool Draw(Leadwort::Components::FieldData& field) {
		if (const auto it = m_Drawers.find(field.type); it != m_Drawers.end()) {
			return it->second(field);
		}

		ImGui::TextDisabled("%s (no drawer)", field.name.c_str());
		return false;
	}

private:
	std::unordered_map<Leadwort::Components::FieldType, DrawerFn> m_Drawers;
};

}