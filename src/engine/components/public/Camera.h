#pragma once

#include "engine/components/Component.h"
#include "engine/core/public/Layers.h"
#include "engine/rendering/public/Skybox.h"
#include "engine/utils/public/Color.h"

#include <glm/glm.hpp>
#include <memory>
#include <variant>

namespace Engine::Core       { class Entity; }
namespace Engine::Components { class Transform; }

namespace Engine::Components {

class Camera : public Component {
public:
	struct SolidColor {
		Utils::Color color;
	};

	struct SkyBox {
		Unique<Rendering::Skybox> skybox;
	};

	std::variant<SolidColor, SkyBox> background = SolidColor(Utils::Color::Gray20());
	float fov         { 60.0f };
	float nearPlane   { 0.1f };
	float farPlane    { 1000.0f };
	float aspect	  { 16.0f / 9.0f };
	uint32_t cullingMask { Utils::Layers::EVERYTHING };

	[[nodiscard]] bool ShouldRender(const Core::Entity& entity) const;
	[[nodiscard]] Mat4 GetViewMatrix() const;
	[[nodiscard]] Mat4 GetProjectionMatrix() const;
};

} // namespace Engine::Components