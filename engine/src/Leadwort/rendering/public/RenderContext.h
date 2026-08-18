#pragma once

#include "Leadwort/core/math/public/Mat4.h"
#include "RenderQueue.h"

namespace Leadwort::Components {
	class Camera;
	class MeshRenderer;
}

namespace Leadwort::Rendering {

	class RenderPassBuilder;

	class RenderContext {
	public:
		Components::Camera* Camera { nullptr };
		RenderQueues* RenderQueues { nullptr };
		Components::MeshRenderer* HighlightedMeshRenderer { nullptr };
		Mat4 LightSpaceMatrix{};
	};

}