#pragma once
#include "../utils/public/Logger.h"
#include "Leadwort/core/public/Core.h"
#include "Leadwort/serialization/ISerializable.h"
#include "Leadwort/utils/public/GizmoShapes.h"

#include <concepts>
#include <gtest/internal/gtest-internal.h>

namespace Leadwort::Core { class Entity; }

namespace Leadwort::Components {

	enum class FieldType {
		Float,
		Vec3,
		Quat,
		Vec4,
		Color,
		Bool,
		Int,
		String,
		AssetRef,
	};

	struct FieldData {
		std::string name{};
		FieldType type{};
		void* dataPtr{};
		std::string displayName{};
		std::string_view assetTypeName{}; // Relevant only if type is AssetRef
	};

	class Component : public Serialization::ISerializable {
	public:
		void SetEntity(Core::Entity& entity) {
			m_Entity = &entity;
		}

		[[nodiscard]] Core::Entity& GetEntity() const {
			LW_ASSERT(m_Entity, "Invalid component: No entity is assigned.");
			return *m_Entity;
		}

		virtual void OnAdded() {}
		virtual void OnRemoved() {}
		virtual void OnFieldsChanged() {}

		virtual void Serialize(Json& out) const override {}
		virtual void Deserialize(const Json& in) override {}
		[[nodiscard]] virtual std::vector<Utils::GizmoShapes::DebugLine> GetGizmoLines() const noexcept { return {}; }

		// Serialization, reflection.
		[[nodiscard]] virtual std::string_view GetTypeName() const override { return "Component"; }
		[[nodiscard]] virtual std::vector<FieldData> GetOwnFields() { return {}; }
		[[nodiscard]] virtual std::vector<FieldData> GetFields() { return {}; }

		#define LW_FIELD(type, member, displayName) FieldData { #member, FieldType::type, &member, displayName }
		#define LW_REFLECT(TypeNameIdentifier, ...)									\
			[[nodiscard]] std::string_view GetTypeName() const final override {   	\
				return #TypeNameIdentifier;                                       	\
			}                                                                     	\
			[[nodiscard]] std::vector<FieldData> GetFields() final override {     	\
				return GetFieldsWithBase(*this);                                  	\
			}                                                                     	\
			[[nodiscard]] std::vector<FieldData> GetOwnFields() override {        	\
				return {                                                          	\
					__VA_ARGS__                                                   	\
				};                                                                	\
			}

	private:
		Core::Entity* m_Entity { nullptr };
	};

	template<typename T>
	std::vector<FieldData> GetFieldsWithBase(T& self) {
		std::vector<FieldData> fields{};

		if constexpr (HasBaseClass<T>) {
			fields = GetFieldsWithBase<typename T::Base>(static_cast<T::Base&>(self));
		}

		auto own = self.T::GetOwnFields();
		fields.insert(fields.end(), own.begin(), own.end());

		return fields;
	}

	template<typename T>
	concept IsComponent = std::derived_from<T, Component> && std::is_default_constructible_v<T>;

} // namespace Engine::Components