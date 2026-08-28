#include <Leadwort/core/public/Game.h>

#include "Leadwort/rendering/public/rendergraph/passes/AlphaTestPass.h"
#include "Leadwort/rendering/public/rendergraph/passes/BackgroundPass.h"
#include "Leadwort/rendering/public/rendergraph/passes/GridPass.h"
#include "Leadwort/rendering/public/rendergraph/passes/OpaquePass.h"
#include "Leadwort/rendering/public/rendergraph/passes/OutlinePass.h"
#include "Leadwort/rendering/public/rendergraph/passes/PostProcessPass.h"
#include "Leadwort/rendering/public/rendergraph/passes/ShadowMapPass.h"
#include "Leadwort/rendering/public/rendergraph/passes/TransparentPass.h"
#include "Leadwort/serialization/SceneSerializer.h"
#include "Leadwort/systems/public/ShaderWatcher.h"

#include <Leadwort/asset-management/public/AssetDatabase.h>
#include <Leadwort/core/public/Time.h>
#include <Leadwort/core/public/Window.h>
#include <Leadwort/systems/public/BehaviourSystem.h>
#include <Leadwort/systems/public/CameraSystem.h>
#include <Leadwort/systems/public/Input.h>
#include <Leadwort/systems/public/RenderSystem.h>
#include <Leadwort/systems/public/SceneSystem.h>

#include <GLFW/glfw3.h>

namespace Leadwort::Core {

	using namespace Rendering::RG;
	using namespace Systems;

	Game::Game() {
		if (!Window::Get().Initialize(1920, 1080, "Leadwort Engine")) {
			LW_ERROR("Error to initialize Engine");
		}

	    Utils::Log::Initialize();
	    Input::Init(Window::Get().GetHandle());

	    RenderSystem::Get().Initialize();
	    SceneSystem::Get().LoadDefaultScene();
	    SceneSystem::Get().LoadPendingScene();

		const int windowWidth { Window::Get().GetWidth() };
		const int windowHeight { Window::Get().GetHeight() };

		m_GameColorTex   = CreateUnique<RenderTexture>(windowWidth, windowHeight, RenderTexture::Format::RGBA16F, "Game Color");
		m_GameDepthTex   = CreateUnique<RenderTexture>(windowWidth, windowHeight, RenderTexture::Format::Depth24Stencil8, "Game Depth");
		m_PostProcessTex = CreateUnique<RenderTexture>(windowWidth, windowHeight, RenderTexture::Format::RGBA16F, "Post Process Color");
		m_SceneColorTex  = CreateUnique<RenderTexture>(windowWidth, windowHeight, RenderTexture::Format::RGBA8, "Scene Color");
		m_SceneDepthTex  = CreateUnique<RenderTexture>(windowWidth, windowHeight, RenderTexture::Format::Depth24Stencil8, "Scene Depth");
		m_ShadowMap		 = CreateUnique<RenderTexture>(2048, 2048, RenderTexture::Format::ShadowDepth32F, "Shadow Map");

		const auto skyHDRI { AssetManagement::EngineAssets::GetTexture("textures/skyboxes/tex_sky.exr") };
		m_IBLResult = CreateUnique<Rendering::IBLBaker::Result>(Rendering::IBLBaker::Bake(*skyHDRI));

		BuildRenderGraphs();
	}

	void Game::BuildRenderGraphs() {
		using namespace Rendering::RG::Passes;

		RenderTexture& gameColor { *m_GameColorTex};
		RenderTexture& gameDepth { *m_GameDepthTex };
		RenderTexture& postColor { *m_PostProcessTex };
		RenderTexture& sceneColor { *m_SceneColorTex };
		RenderTexture& sceneDepth { *m_SceneDepthTex };
		RenderTexture& shadowMap { *m_ShadowMap };
		const Rendering::IBLBaker::Result& ibl { *m_IBLResult };

		m_GameRenderGraph.AddPass<BackgroundPass>(gameColor, gameDepth);
		m_GameRenderGraph.AddPass<ShadowMapPass>(shadowMap);
		m_GameRenderGraph.AddPass<OpaquePass>(gameColor, gameDepth, shadowMap, ibl);
		m_GameRenderGraph.AddPass<AlphaTestPass>(gameColor, gameDepth, ibl);
		m_GameRenderGraph.AddPass<TransparentPass>(gameColor, gameDepth, ibl);
		m_GameRenderGraph.AddPass<PostProcessPass>(gameColor, postColor, gameDepth);
		m_GameRenderGraph.Compile();

		m_SceneRenderGraph.AddPass<BackgroundPass>(sceneColor, sceneDepth);
		m_SceneRenderGraph.AddPass<ShadowMapPass>(shadowMap);
		m_SceneRenderGraph.AddPass<OpaquePass>(sceneColor, sceneDepth, shadowMap, ibl);
		m_SceneRenderGraph.AddPass<AlphaTestPass>(sceneColor, sceneDepth, ibl);
		m_SceneRenderGraph.AddPass<GridPass>(sceneColor, sceneDepth);
		m_SceneRenderGraph.AddPass<TransparentPass>(sceneColor, sceneDepth, ibl);
		m_SceneRenderGraph.AddPass<OutlinePass>(sceneColor, sceneDepth);
		m_SceneRenderGraph.Compile();
	}

	void Game::Tick() const {
	    SceneSystem::Get().LoadPendingScene();

		if (Input::Keyboard::IsOrderedCombo(Key::LeftControl, Key::R)) {
			ShaderWatcher::Get().MarkAllShaderAsPending();
			ShaderWatcher::Get().RecompilePendingShaders();
		}

		if (Input::Keyboard::IsOrderedCombo(Key::LeftControl, Key::S)) {
			const std::string path = AssetManagement::EngineAssets::ResolvePath("scenes/") + "/empty-scene.json";
			Serialization::SceneSerializer::SaveToFile(*SceneSystem::Get().GetCurrentScene(), path);
		}

	    Time::Update(glfwGetTime());
	    Input::Update(Time::GetDeltaTime());

	    while (Time::ShouldRunFixedUpdate()) {
	        BehaviourSystem::Get().FixedUpdate();
	        Time::ConsumeFixedUpdate();
	    }

	    BehaviourSystem::Get().Update();

		// A scene can legitimately be left without one of these — the user just deleted the
		// entity that carried it — and a missing camera means nothing to draw, not a crash.
		if (auto* gameCamera = CameraSystem::Get().GetMainCamera()) {
			RenderSystem::Get().Render(*gameCamera, m_GameRenderGraph);
		}

		if (auto* sceneCamera = CameraSystem::Get().GetSceneCamera()) {
			RenderSystem::Get().Render(*sceneCamera, m_SceneRenderGraph);
		}

		RenderSystem::Get().ClearScreen();
	}

	void Game::Loop(const std::function<void()>& preTick, const std::function<void()>& renderOverlay) const {
		Utils::Log::Header("Hello World!");

		while (Window::Get().IsOpen()) {
			Window::PollEvents();

			if (preTick) {
				preTick();
			}

			Tick();

			if (renderOverlay) {
				renderOverlay();
			}

			Window::Get().SwapBuffers();
		}

		Utils::Log::Header("Bye bye!");
	}

	void Game::ResizeGameView(const int width, const int height) {
		m_GameColorTex->Resize(width, height);
		m_GameDepthTex->Resize(width, height);
		m_PostProcessTex->Resize(width, height);

		m_GameRenderGraph.Compile();

		if (auto* gameCam = CameraSystem::Get().GetMainCamera()) {
			gameCam->Aspect = static_cast<float>(width) / static_cast<float>(height);
		}
	}

	void Game::ResizeSceneView(const int width, const int height) {
		m_SceneColorTex->Resize(width, height);
		m_SceneDepthTex->Resize(width, height);

		m_SceneRenderGraph.Compile();

		if (auto* sceneCam = CameraSystem::Get().GetSceneCamera()) {
			sceneCam->Aspect = static_cast<float>(width) / static_cast<float>(height);
		}
	}

	void Game::SetHighlightedEntity(const EntityID entityID) {
		RenderSystem& renderSystem { RenderSystem::Get() };

		const IScene* scene { SceneSystem::Get().GetCurrentScene() };
		Entity* entity { nullptr };

		if (scene != nullptr && entityID != Entity::ROOT_ENTITY_ID) {
			const auto& entityMap { scene->GetEntityMap() };

			if (const auto it { entityMap.find(entityID) }; it != entityMap.end()) {
				entity = it->second.get();
			}
		}

		if (entity == nullptr || !entity->HasComponent<Components::MeshRenderer>()) {
			renderSystem.SetHighlightedEntity(nullptr);
			return;
		}

		renderSystem.SetHighlightedEntity(entity);
	}

} // namespace Engine::Core