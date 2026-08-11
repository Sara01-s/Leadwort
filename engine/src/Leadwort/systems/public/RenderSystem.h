#pragma once

#include "Leadwort/rendering/public/rendergraph/RenderGraph.h"

#include <Leadwort/components/public/Camera.h>
#include <Leadwort/core/math/public/Color.h>
#include <Leadwort/rendering/public/CameraUBO.h>
#include <Leadwort/rendering/public/LightingUBO.h>
#include <Leadwort/rendering/public/PostProcess.h>
#include <Leadwort/rendering/public/SceneCollector.h>
#include <Leadwort/utils/public/Singleton.h>

namespace Leadwort::Systems {

class RenderSystem : public Utils::Singleton<RenderSystem> {
    friend class Singleton;
	using Shader = Rendering::Bindables::Shader;

public:
    void Initialize();

	void Render(Components::Camera& camera, const Rendering::RG::RenderGraph& graph) const;
	void SetHighlightedEntity(Core::Entity* entity) noexcept { m_HighlightedEntity = entity; }

	static void ClearScreen();
    static void SetClearColor(Color color);

private:
	RenderSystem() = default;
    ~RenderSystem() = default;

private:
	Rendering::CameraUBO m_CameraUBO{};
	Rendering::LightingUBO m_LightingUBO{};
	Rendering::SceneCollector m_SceneCollector{};
	Rendering::RG::RenderGraph m_GameRenderGraph{};
	Rendering::RG::RenderGraph m_SceneRenderGraph{};

	Core::Entity* m_HighlightedEntity { nullptr };
};

} // namespace Engine::Systems