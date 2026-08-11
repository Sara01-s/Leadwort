#pragma once

#include "RenderQueue.h"

#include <vector>

namespace Leadwort::Core       { class Scene; }
namespace Leadwort::Components { class Camera; class IRenderer; }

namespace Leadwort::Rendering {

class SceneCollector {
public:
	void FindRenderersInScene(const Core::Scene& scene);
	[[nodiscard]] RenderQueues BuildRenderQueues(const Components::Camera& camera) const;

private:
	std::vector<Components::IRenderer*> m_Renderers;
};

} // namespace Engine::Rendering