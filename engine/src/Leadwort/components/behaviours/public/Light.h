#pragma once

#include "Leadwort/components/ComponentRegistry.h"
#include "Leadwort/components/public/Transform.h"

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

        void Serialize(Json& out) const override {
          out["type"] = static_cast<int>(Type);
          out["intensity"] = Intensity;
          out["color"] = Color;
          out["attenuation"] = Attenuation;
          out["innerCutoff"] = InnerCutoff;
          out["outerCutoff"] = OuterCutoff;
       }

        void Deserialize(const Json& in) override {
          Type = static_cast<LightType>(in.value("type", static_cast<int>(Type)));
          Intensity = in.value("intensity", Intensity);
          Color = in.value("color", Color);
          Attenuation = in.value("attenuation", Attenuation);
          InnerCutoff = in.value("innerCutoff", InnerCutoff);
          OuterCutoff = in.value("outerCutoff", OuterCutoff);
       }

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

		static float CalculateLightRange(const float intensity, const Vec3& attenuation) noexcept {
			constexpr float threshold { 0.01f };
			const float maxIntensity { std::max(intensity, 1.0f) };
			const float K { maxIntensity / threshold };

			const float c { attenuation.x };
			const float l { attenuation.y };
			const float q { attenuation.z };

			if (q > 0.0001f) {
				const float discriminant { l * l - 4.0f * q * (c - K) };
				if (discriminant >= 0.0f) {
					return (-l + std::sqrt(discriminant)) / (2.0f * q);
				}
			}
			else if (l > 0.0001f) {
				return (K - c) / l;
			}

			return 10.0f; // Fallback
		}

		[[nodiscard]]
		std::vector<Utils::GizmoShapes::DebugLine> GetGizmoLines() const noexcept override {
	        std::vector<Utils::GizmoShapes::DebugLine> lines{};

	        const auto& transform = GetEntity().GetTransform();
	        const Vec3 pos = transform.GetLocalPosition();
	        const Vec3 dir = transform.GetForward().Normalized();

	        switch (Type) {
	            case LightType::Directional: {
	                constexpr float headRadius { 0.6f };
	                constexpr float rayLength { 2.5f };
	                constexpr int segments { 12 };

	                auto circleLines { Utils::GizmoShapes::Circle(pos, dir, headRadius, segments) };
	                lines.insert(lines.end(), circleLines.begin(), circleLines.end());

	                const Vec3 ref { (std::abs(Dot(dir, Vec3::Up())) > 0.99f) ? Vec3::Right() : Vec3::Up() };
	                const Vec3 tangent { Cross(dir, ref).Normalized() };
	                const Vec3 bitangent { Cross(dir, tangent).Normalized() };

	                lines.push_back({ pos, pos + dir * rayLength });
	                lines.push_back({ pos + tangent * headRadius, pos + tangent * headRadius + dir * rayLength });
	                lines.push_back({ pos - tangent * headRadius, pos - tangent * headRadius + dir * rayLength });
	                lines.push_back({ pos + bitangent * headRadius, pos + bitangent * headRadius + dir * rayLength });
	                lines.push_back({ pos - bitangent * headRadius, pos - bitangent * headRadius + dir * rayLength });
	                break;
	            }
	            case LightType::Point: {
	                const float range { CalculateLightRange(Intensity, Attenuation) };

	                // Center
	                auto centerSphere { Utils::GizmoShapes::Sphere(pos, 0.25f, 8) };
	                auto rangeSphere { Utils::GizmoShapes::Sphere(pos, range, 32) };

	                lines.reserve(centerSphere.size() + rangeSphere.size());
	                lines.insert(lines.end(), centerSphere.begin(), centerSphere.end());
	                lines.insert(lines.end(), rangeSphere.begin(), rangeSphere.end());
	                break;
	            }
	            case LightType::Spot: {
	                const float range { CalculateLightRange(Intensity, Attenuation) };

	                // Outer Cutoff
	                const float outerAngleRad { OuterCutoff * DegToRad };
	                auto outerCone { Utils::GizmoShapes::Cone(pos, dir, outerAngleRad, range, 32) };
	                lines.insert(lines.end(), outerCone.begin(), outerCone.end());

	                // Inner Cutoff
	                if (InnerCutoff < OuterCutoff && InnerCutoff > 0.0f) {
	                    const float innerAngleRad { InnerCutoff * DegToRad };
	                    auto innerCone { Utils::GizmoShapes::Cone(pos, dir, innerAngleRad, range, 12) };
	                    lines.insert(lines.end(), innerCone.begin(), innerCone.end());
	                }
	                break;
	            }
	        }

	        return lines;
	    }

		[[nodiscard]]
		static Mat4 CalculateLightSpaceMatrix(const Light& light) noexcept {
			constexpr float orthoHalfSize { 20.0f };
			constexpr float nearPlane { 0.1f };
			constexpr float farPlane  { 100.0f };
			constexpr float shadowDistance { 30.0f };
			constexpr Vec3 sceneCenter { 0.0f, 0.0f, 0.0f };

			const Vec3 lightDir = light.GetEntity().GetTransform().GetForward().Normalized();
			const Vec3 lightPos = sceneCenter - lightDir * shadowDistance;

			const Mat4 lightView = Mat4::LookAt(lightPos, sceneCenter, Vec3::Up());
			const Mat4 lightProjection = Mat4::Orthographic(
				-orthoHalfSize, orthoHalfSize,
				-orthoHalfSize, orthoHalfSize,
				nearPlane, farPlane
			);

			return lightProjection * lightView;
		}
	};

	LW_REGISTER_COMPONENT(Light)
}