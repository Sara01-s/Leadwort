#pragma once

#include "Leadwort/rendering/IBLBaker.h"

#include <Leadwort/core/public/Core.h>
#include <Leadwort/rendering/public/rendergraph/RenderGraph.h>

#include <memory>

namespace Leadwort::Core {

	class Game {
	public:
		Game();
		void BuildRenderGraphs();

		void Tick() const;
		void Loop(const std::function<void()>& preTick, const std::function<void()>& renderOverlay) const;

		void ResizeGameView(int width, int height);
		void ResizeSceneView(int width, int height);
		static void SetHighlightedEntity(EntityID entityID);

		[[nodiscard]] Rendering::RG::RenderTexture& GetGameColorTexture()   const noexcept { return *m_GameColorTex; }
		[[nodiscard]] Rendering::RG::RenderTexture& GetGameDepthTexture()   const noexcept { return *m_GameDepthTex; }
		[[nodiscard]] Rendering::RG::RenderTexture& GetGameOutputTexture()  const noexcept { return *m_PostProcessTex; }
		[[nodiscard]] Rendering::RG::RenderTexture& GetSceneOutputTexture() const noexcept { return *m_SceneColorTex; }
		[[nodiscard]] Rendering::RG::RenderTexture& GetSceneDepthTexture()  const noexcept { return *m_SceneDepthTex; }

		[[nodiscard]] Rendering::RG::RenderGraph& GetGameRenderGraph()  noexcept { return m_GameRenderGraph; }
		[[nodiscard]] Rendering::RG::RenderGraph& GetSceneRenderGraph() noexcept { return m_SceneRenderGraph; }

	private:
		Unique<Rendering::RG::RenderTexture> m_GameColorTex{};
		Unique<Rendering::RG::RenderTexture> m_GameDepthTex{};
		Unique<Rendering::RG::RenderTexture> m_PostProcessTex{};
		Unique<Rendering::RG::RenderTexture> m_SceneColorTex{};
		Unique<Rendering::RG::RenderTexture> m_SceneDepthTex{};
		Unique<Rendering::RG::RenderTexture> m_ShadowMap{};
		Unique<Rendering::IBLBaker::Result> m_IBLResult;

		Rendering::RG::RenderGraph m_GameRenderGraph  { "Game Render Graph" };
		Rendering::RG::RenderGraph m_SceneRenderGraph { "Scene Render Graph" };
	};

} // namespace Engine::Core