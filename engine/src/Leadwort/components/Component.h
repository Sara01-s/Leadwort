#pragma once
#include "../utils/public/Logger.h"
#include "Leadwort/serialization/ISerializable.h"

#include <concepts>

namespace Leadwort::Core { class Entity; }

namespace Leadwort::Components {

enum class FieldType {
	Float,
	Vec3,
	Vec4,
	Color,
	Bool,
	Int,
	String,
	AssetRef
};

struct FieldInfo {
	std::string name{};
	FieldType type{};
	void* ptr;
};

class Component : public Serialization::ISerializable {
public:
	void SetEntity(Core::Entity& entity) {
		m_Entity = &entity;
	}

	[[nodiscard]] Core::Entity& GetEntity() const {
		LW_ASSERT(m_Entity, "Invalid component: No entity assigned.");
		return *m_Entity;
	}

	virtual void OnAdded() {}
	virtual void OnRemoved() {}

	virtual void Serialize(Json& out) const override {}
	virtual void Deserialize(const Json& in) override {}
	virtual std::string_view GetTypeName() const override { return "Component"; }
	virtual std::vector<FieldInfo> GetFields() const { return {}; }

private:
	Core::Entity* m_Entity { nullptr };
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

template <typename T> using BaseOf_t = BaseOf<T>::type;
} // namespace Engine::Components