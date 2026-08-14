#pragma once

#include "../../core/math/public/Color.h"
#include "../../core/public/Layers.h"
#include "../../rendering/public/Skybox.h"
#include "../Component.h"
#include "Leadwort/core/math/public/Ray.h"

#include <memory>
#include <variant>

namespace Leadwort::Core       { class Entity; }
namespace Leadwort::Components { class Transform; }

namespace Leadwort::Components {

	class Camera : public Component {
	public:
		LW_REFLECT(Camera,
			LW_FIELD(Float, fov, "Fov"),
			LW_FIELD(Float, nearPlane, "Near"),
			LW_FIELD(Float, farPlane, "Far"),
		)

		struct SolidColor {
			Color color;
		};

		struct SkyBox {
			Unique<Rendering::Skybox> skybox;
		};

		std::variant<SolidColor, SkyBox> background = SolidColor(Color::Gray20());
		float fov         { 60.0f };
		float nearPlane   { 0.1f };
		float farPlane    { 1000.0f };
		float aspect	  { 16.0f / 9.0f };
		uint32_t cullingMask { Utils::Layers::EVERYTHING };

		[[nodiscard]] Vec3 WorldToCameraSpace(const Vec3& worldPosition) const noexcept;

		[[nodiscard]] bool ShouldRender(const Core::Entity& entity) const noexcept;
		[[nodiscard]] Mat4 GetViewMatrix() const noexcept;
		[[nodiscard]] Mat4 GetProjectionMatrix() const noexcept;
		[[nodiscard]] Ray ScreenPointToRay(const Vec2& normalizedScreenPoint) const noexcept;

		// Camera.cpp
		std::vector<Utils::GizmoShapes::DebugLine> GetGizmoLines() const noexcept override;
	};

} // namespace Engine::Components