#pragma once
#include "engine/components/Component.h"
#include "engine/rendering/public/RenderQueue.h"

namespace Engine::Rendering {
	class DrawCommandBuffer;
}

namespace Engine::Components {

class Camera;

class Renderer : public Component {
public:
	~Renderer() override = default;

	Rendering::RenderQueue renderQueue = Rendering::RenderQueue::Opaque;
	bool isVisible = true;

	virtual void EmitDrawCommand(Rendering::DrawCommandBuffer& drawCmdBuffer, const Camera& camera) const = 0;
};

} // namespace Engine::Components