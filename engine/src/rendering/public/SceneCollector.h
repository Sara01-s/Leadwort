#pragma once

#include "RenderQueue.h"

#include <vector>

namespace Engine::Core       { class Scene; }
namespace Engine::Components { class Camera; class Renderer; }

namespace Engine::Rendering {

class SceneCollector {
public:
	void FindRenderersInScene(const Core::Scene& scene);
	[[nodiscard]] RenderQueues BuildRenderQueues(const Components::Camera& camera) const;

private:
	std::vector<Components::Renderer*> m_Renderers;
};

} // namespace Engine::Rendering