#pragma once

#include "../../core/math/public/Color.h"
#include "../../core/public/Layers.h"
#include "../../rendering/public/Skybox.h"
#include "../Component.h"
#include "Leadwort/components/ComponentRegistry.h"
#include "Leadwort/core/math/public/Ray.h"

#include <memory>
#include <variant>

namespace Leadwort::Core       { class Entity; }
namespace Leadwort::Components { class Transform; }

namespace Leadwort::Components {

	class Camera : public Component {
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
		std::variant<SolidColor, SkyBox> Background = SolidColor(Color::Gray20());

		float Fov         { 60.0f };
		float NearPlane   { 0.1f };
		float FarPlane    { 1000.0f };
		float Aspect	  { 16.0f / 9.0f };
		uint32_t CullingMask { Utils::Layers::EVERYTHING };

	public:
		[[nodiscard]] Vec3 WorldToCameraSpace(const Vec3& worldPosition) const noexcept;

		[[nodiscard]] bool ShouldRender(const Core::Entity& entity) const noexcept;
		[[nodiscard]] Mat4 GetViewMatrix() const noexcept;
		[[nodiscard]] Mat4 GetProjectionMatrix() const noexcept;
		[[nodiscard]] Ray ScreenPointToRay(const Vec2& normalizedScreenPoint) const noexcept;

		// Camera.cpp
		std::vector<Utils::GizmoShapes::DebugLine> GetGizmoLines() const noexcept override;
	};

	LW_REGISTER_COMPONENT(Camera)

} // namespace Engine::Components