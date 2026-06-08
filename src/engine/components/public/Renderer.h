#pragma once
#include "engine/components/Component.h"
#include "engine/rendering/public/RenderQueue.h"

namespace Engine::Components {

class Camera;

class Renderer : public Component {
public:
	~Renderer() override = default;

	Rendering::RenderQueue renderQueue = Rendering::RenderQueue::Opaque;
	bool isVisible = true;

	virtual void Render(const Camera* camera) = 0;
};

} // namespace Engine::Components