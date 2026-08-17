#pragma once

#include "Leadwort/components/ComponentRegistry.h"

#include <Leadwort/components/behaviours/public/Behaviour.h>
#include <Leadwort/core/math/public/Color.h>
#include <Leadwort/systems/public/LightingSystem.h>

namespace Leadwort::Components::Behaviours {

	class Light : public Behaviour {
	public:
		enum class LightType : int { Directional, Point, Spot };

	public:
		LightType Type { LightType::Directional };
		float Intensity { 5.0f };
		Color Color { Color::White() };

		Vec3 Attenuation { 1.0f, 0.09f, 0.032f };

		float InnerCutoff { 12.5f };
		float OuterCutoff { 17.5f };

		LW_REFLECT(Light,
			LW_FIELD_ENUM(Type, "Light Type", "Directional", "Point", "Spot"),
			LW_FIELD(Float, Intensity, "Intensity"),
			LW_FIELD(Color, Color, "Color"),
			LW_FIELD(Vec3, Attenuation, "Attenuation (C, L, Q)"),
			LW_FIELD(Float, InnerCutoff, "Inner Cutoff"),
			LW_FIELD(Float, OuterCutoff, "Outer Cutoff")
		)
	public:
		[[nodiscard]]
		std::string_view GetTypeAsString() const noexcept {
			switch (Type) {
				case LightType::Directional: return "Directional";
				case LightType::Point: return "Point";
				case LightType::Spot: return "Spot";
				default: return "Unknown (ERROR)";
			}
		}

		void OnEnable() override {
			Systems::LightingSystem::Get().Register(this);
		}

		void OnDisable() override {
			Systems::LightingSystem::Get().Unregister(this);
		}
	};

	LW_REGISTER_COMPONENT(Light)
}