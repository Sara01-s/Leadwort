#pragma once
#include <Leadwort/components/Component.h>
#include <Leadwort/rendering/public/RenderQueue.h>
#include <Leadwort/core/math/public/AABB.h>

#include <optional>

namespace Leadwort::Rendering { class DrawCommandBuffer; }

namespace Leadwort::Components {

class Camera;

class IRenderer : public Component {
public:
	~IRenderer() override = default;

	Rendering::RenderQueue renderQueue = Rendering::RenderQueue::Opaque;
	bool isVisible = true;

	virtual void EmitDrawCommand(Rendering::DrawCommandBuffer& drawCmdBuffer, const Camera& camera) const = 0;
	[[nodiscard]] virtual std::optional<AABB> GetAABB() const { return std::nullopt; }
};

} // namespace Engine::Components