#pragma once

#include "engine/asset-management/public/AssetManager.h"
#include "engine/components/public/Camera.h"
#include "engine/components/public/Renderer.h"
#include "engine/components/public/Transform.h"
#include "engine/rendering/bindables/public/RenderTarget.h"
#include "engine/rendering/public/CameraUBO.h"
#include "engine/rendering/public/PostProcess.h"
#include "engine/rendering/public/SceneCollector.h"
#include "engine/utils/public/Color.h"
#include "engine/utils/public/Singleton.h"

#include <functional>
#include <memory>
#include <vector>

namespace Engine::Systems {

class RenderSystem : public Utils::Singleton<RenderSystem> {
    friend class Singleton;

public:
    void Init();

	void Render(Components::Camera* camera, const Rendering::Bindables::RenderTarget* renderTarget) const;
	void RenderPostProcess(const Rendering::Bindables::RenderTarget* src,
						   const Rendering::Bindables::RenderTarget* dst) const;
    void RenderUI() const;
    void AddOverlay(std::function<void()> callback);

	static void Clear();
	static void ClearScreen();
    static void SetClearColor(const Utils::Color& color);
    static void SetClearColor(float r, float g, float b, float a = 1.0f);

private:
	RenderSystem() = default;
    ~RenderSystem();

	static void ApplySceneState();
	static void ApplyPostProcessState();
	static void RenderBackground(const Components::Camera* camera, Vec2 resolution);
	static void RenderOpaque(const Rendering::RenderQueues& queues, const Components::Camera* camera);
	static void RenderAlphaTest(const Rendering::RenderQueues& queues, const Components::Camera* camera);
	static void RenderTransparent(Rendering::RenderQueues& queues, const Components::Camera* camera);
	static void SortTransparents(std::vector<Components::Renderer*>& transparents, const Vec3& cameraPosition);
	void RenderGrid(const Components::Camera* camera, Vec2 resolution) const;

	Shared<Rendering::Bindables::Shader> m_GridShader = AssetManagement::EngineAssets::GetShader("shaders/shd_grid.glsl");
	GLuint m_EmptyVAO;

	Rendering::CameraUBO m_CameraUBO{};
	Rendering::SceneCollector m_SceneCollector{};

    std::vector<std::function<void()>> m_OverlayCallbacks;
    Unique<Rendering::PostProcess> m_PostProcess;
};

} // namespace Engine::Systems