#pragma once

#include "RenderQueue.h"

#include <vector>

namespace Leadwort::Core       { class Scene; }
namespace Leadwort::Components { class Camera; class Renderer; }

namespace Leadwort::Rendering {

class SceneCollector {
public:
	void FindRenderersInScene(const Core::Scene& scene);
	[[nodiscard]] RenderQueues BuildRenderQueues(const Components::Camera& camera) const;

private:
	std::vector<Components::Renderer*> m_Renderers;
};

} // namespace Engine::Rendering