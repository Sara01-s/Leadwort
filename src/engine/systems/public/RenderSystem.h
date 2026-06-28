#pragma once

#include "engine/components/public/Camera.h"
#include "engine/components/public/Transform.h"
#include "engine/rendering/bindables/public/RenderTarget.h"
#include "engine/rendering/public/CameraUBO.h"
#include "engine/rendering/public/PostProcess.h"
#include "engine/rendering/public/RenderPass.h"
#include "engine/rendering/public/SceneCollector.h"
#include "engine/utils/public/Color.h"
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

	void Render(Components::Camera& camera, const Rendering::Bindables::RenderTarget& renderTarget) const;
    void RenderUI() const;
    void AddOverlay(std::function<void()> callback);

	static void ClearScreen();
    static void SetClearColor(const Utils::Color& color);
    static void SetClearColor(float r, float g, float b, float a = 1.0f);

private:
	RenderSystem() = default;
    ~RenderSystem() = default;

private:
	Rendering::CameraUBO m_CameraUBO{};
	Rendering::SceneCollector m_SceneCollector{};

	std::vector<Unique<Rendering::RenderPass>> m_RenderPasses{};
    std::vector<std::function<void()>> m_OverlayCallbacks{};
    Unique<Rendering::PostProcess> m_PostProcess{};
};

} // namespace Engine::Systems