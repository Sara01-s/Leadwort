#pragma once

#include "engine/components/Component.h"
#include "engine/core/public/Layers.h"
#include "engine/core/public/Scene.h"
#include "engine/rendering/public/MatrixUtils.h"
#include "engine/rendering/public/Skybox.h"
#include "engine/utils/public/Color.h"

#include <memory>
#include <variant>

#undef near
#undef far

namespace Engine::Components {

class Camera : public Component {
public:
	struct SolidColor {
		Utils::Color color;
	};

	struct SkyBox {
		std::unique_ptr<Rendering::Skybox> skybox;
	};

	using Background = std::variant<SolidColor, SkyBox>;

	uint32_t cullingMask = Utils::Layers::EVERYTHING;
	Background background = SolidColor{Utils::Color::Gray30()};

	float fov = 45.0f;
	float near = 0.01f;
	float far = 1000.0f;
	float aspect = 16.0f / 9.0f; // Set by RenderSystem each frame.

	[[nodiscard]] bool ShouldRender(const Core::Entity* entity) const {
		return (entity->layerMask & cullingMask) != 0;
	}

	[[nodiscard]] glm::mat4 GetViewMatrix() const {
		return Rendering::MatrixUtils::CalculateViewMatrix(*entity->transform);
	}

	[[nodiscard]] glm::mat4 GetProjectionMatrix() const {
		return Rendering::MatrixUtils::CalculateProjectionMatrix(fov, near, far, aspect);
	}
};

} // namespace Engine::Components