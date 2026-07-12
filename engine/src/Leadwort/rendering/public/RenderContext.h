#pragma once
#include "../../components/public/Camera.h"
#include "RenderQueue.h"

namespace Leadwort::Rendering {

class RenderPassBuilder;

class RenderContext {
public:
	Components::Camera* camera { nullptr };
	RenderQueues* renderQueues { nullptr };
};

}