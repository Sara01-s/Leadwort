#pragma once

#include "../../core/math/public/Color.h"
#include "engine/components/public/Camera.h"
#include "engine/rendering/public/CameraUBO.h"
#include "engine/rendering/public/LightingUBO.h"
#include "engine/rendering/public/PostProcess.h"
#include "engine/rendering/public/RenderGraph.h"
#include "engine/rendering/public/SceneCollector.h"
#include "engine/utils/public/Singleton.h"

#include <functional>
#include <memory>
#include <vector>

namespace Engine::Systems {

class RenderSystem : public Utils::Singleton<RenderSystem> {
    friend class Singleton;
	using Shader = Rendering::Bindables::Shader;

public:
    void Initialize();

	void Render(Components::Camera& camera, const Rendering::RenderGraph& graph) const;
    void RenderUI() const;
    void AddOverlayCallback(std::function<void()> callback);

	static void ClearScreen();
    static void SetClearColor(Color color);

private:
	RenderSystem() = default;
    ~RenderSystem() = default;

private:
	Rendering::CameraUBO m_CameraUBO{};
	Rendering::LightingUBO m_LightingUBO{};
	Rendering::SceneCollector m_SceneCollector{};
	Rendering::RenderGraph m_GameRenderGraph{};
	Rendering::RenderGraph m_SceneRenderGraph{};

    std::vector<std::function<void()>> m_OverlayCallbacks{};
};

} // namespace Engine::Systems