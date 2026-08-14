#pragma once

#include "RenderQueue.h"

namespace Leadwort::Components {
	class Camera;
	class MeshRenderer;
}

namespace Leadwort::Rendering {

	class RenderPassBuilder;

	class RenderContext {
	public:
		Components::Camera* camera { nullptr };
		RenderQueues* renderQueues { nullptr };
		Components::MeshRenderer* highlightedMeshRenderer { nullptr };
	};

}