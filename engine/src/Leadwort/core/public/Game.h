#pragma once

#include "../../rendering/public/RenderGraph.h"
#include "Core.h"

#include <memory>

namespace Leadwort::Core {

class Game {
public:
	Game();
	void BuildRenderGraphs();

	void Tick() const;
	void Loop(const std::function<void()>& renderOverlay) const;

	void ResizeGameView(int width, int height);
	void ResizeSceneView(int width, int height);
	static void SetHighlightedEntity(EntityID entityID);

	[[nodiscard]] Rendering::RenderTexture& GetGameColorTexture()  const noexcept { return *m_GameColorTex; }
	[[nodiscard]] Rendering::RenderTexture& GetGameDepthTexture()  const noexcept { return *m_GameDepthTex; }
	[[nodiscard]] Rendering::RenderTexture& GetGameOutputTexture() const noexcept { return *m_PostProcessTex; }
	[[nodiscard]] Rendering::RenderTexture& GetSceneOutputTexture() const noexcept { return *m_SceneColorTex; }
	[[nodiscard]] Rendering::RenderTexture& GetSceneDepthTexture() const noexcept { return *m_SceneDepthTex; }

private:
	Unique<Rendering::RenderTexture> m_GameColorTex{};
	Unique<Rendering::RenderTexture> m_GameDepthTex{};
	Unique<Rendering::RenderTexture> m_PostProcessTex{};
	Unique<Rendering::RenderTexture> m_SceneColorTex{};
	Unique<Rendering::RenderTexture> m_SceneDepthTex{};

	Rendering::RenderGraph m_GameRenderGraph{};
	Rendering::RenderGraph m_SceneRenderGraph{};
};

} // namespace Engine::Core