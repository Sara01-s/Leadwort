#pragma once

#include "Leadwort/components/ComponentRegistry.h"
#include "Leadwort/core/math/public/Color.h"
#include "Leadwort/core/math/public/Ray.h"
#include "Leadwort/rendering/public/Skybox.h"

#include <memory>
#include <variant>

namespace Leadwort::Core       { class Entity; }
namespace Leadwort::Components { class Transform; }

namespace Leadwort::Components {

	class Camera final : public Component {
	public:
		LW_REFLECT(Camera,
			LW_FIELD(Float, Fov, "Fov"),
			LW_FIELD(Float, NearPlane, "Near"),
			LW_FIELD(Float, FarPlane, "Far"),
		)

		struct SolidColor {
			Color Color;
		};

		struct SkyBox {
			Unique<Rendering::Skybox> Sky;
		};

	public:
		std::variant<SolidColor, SkyBox> Background { SolidColor(Color::Gray20()) };

		float Fov         { 60.0f };
		float NearPlane   { 0.1f };
		float FarPlane    { 1000.0f };
		float Aspect	  { 16.0f / 9.0f };
		uint32_t CullingMask { Utils::Layers::EVERYTHING };

	public:
		void Serialize(Json& out) const override {
			out["fov"] = Fov;
			out["near"] = NearPlane;
			out["far"] = FarPlane;
			out["aspect"] = Aspect;
			out["cullingMask"] = CullingMask;

			if (auto* skybox { std::get_if<SkyBox>(&Background) }) {
				out["backgroundType"] = "SkyBox";
				out["skyboxExrPath"] = skybox->Sky->GetExrPath();
				out["skyboxExposure"] = skybox->Sky->GetExposure();
				out["skyboxTint"] = skybox->Sky->GetTint();
				out["skyboxRotation"] = skybox->Sky->GetRotation();
			}
			else if (auto* solidColor { std::get_if<SolidColor>(&Background) }) {
				out["backgroundType"] = "SolidColor";
				out["backgroundColor"] = solidColor->Color;
			}
		}

		void Deserialize(const Json& in) override {
			Fov         = in.value("fov", Fov);
			NearPlane   = in.value("near", NearPlane);
			FarPlane    = in.value("far", FarPlane);
			Aspect      = in.value("aspect", Aspect);
			CullingMask = in.value("cullingMask", CullingMask);

			const std::string backgroundType { in.value("backgroundType", std::string { "SolidColor" }) };

			if (backgroundType == "SkyBox") {
				auto skybox { CreateUnique<Rendering::Skybox>(in.value("skyboxExrPath", std::string {})) };
				skybox->SetExposure(in.value("skyboxExposure", 1.0f));
				skybox->SetTint(in.value("skyboxTint", Vec3(1.0f, 1.0f, 1.0f)));
				skybox->SetRotation(in.value("skyboxRotation", 0.0f));
				Background = SkyBox { .Sky = std::move(skybox) };
			}
			else {
				Background = SolidColor { .Color = in.value("backgroundColor", Color::Gray20()) };
			}
		}

		[[nodiscard]] Vec3 WorldToCameraSpace(const Vec3& worldPosition) const noexcept;

		[[nodiscard]] bool ShouldRender(const Core::Entity& entity) const noexcept;
		[[nodiscard]] Mat4 GetViewMatrix() const noexcept;
		[[nodiscard]] Mat4 GetProjectionMatrix() const noexcept;
		[[nodiscard]] Ray ScreenPointToRay(const Vec2& normalizedScreenPoint) const noexcept;

		std::vector<Utils::GizmoShapes::DebugLine> GetGizmoLines() const noexcept override;
	};

	LW_REGISTER_COMPONENT(Camera)

} // namespace Engine::Components