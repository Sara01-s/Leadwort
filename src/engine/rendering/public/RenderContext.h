#pragma once
#include "RenderQueue.h"
#include "engine/components/public/Camera.h"

namespace Engine::Rendering {

class RenderPassBuilder;

class RenderContext {
public:
	Components::Camera* camera { nullptr };
	RenderQueues* renderQueues { nullptr };
};

}