#include <Leadwort/core/public/Game.h>

#include "Leadwort/rendering/public/rendergraph/passes/AlphaTestPass.h"
#include "Leadwort/rendering/public/rendergraph/passes/BackgroundPass.h"
#include "Leadwort/rendering/public/rendergraph/passes/GridPass.h"
#include "Leadwort/rendering/public/rendergraph/passes/OpaquePass.h"
#include "Leadwort/rendering/public/rendergraph/passes/TransparentPass.h"
#include "Leadwort/rendering/public/rendergraph/passes/PostProcessPass.h"
#include "Leadwort/rendering/public/rendergraph/passes/OutlinePass.h"
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
		if (!Window::Get().Initialize(1920, 1080, "Leadwort")) {
			LW_ERROR("Error to initialize Engine");
		}

	    Utils::Log::Initialize();
	    Input::Init(Window::Get().GetHandle());

	    RenderSystem::Get().Initialize();
	    SceneSystem::Get().LoadEmptyScene();
	    SceneSystem::Get().LoadPendingScene();

		const int windowWidth = Window::Get().GetWidth();
		const int windowHeight = Window::Get().GetHeight();

		m_GameColorTex   = CreateUnique<RenderTexture>(windowWidth, windowHeight, RenderTexture::Format::RGBA16F);
		m_GameDepthTex   = CreateUnique<RenderTexture>(windowWidth, windowHeight, RenderTexture::Format::Depth24Stencil8);
		m_PostProcessTex = CreateUnique<RenderTexture>(windowWidth, windowHeight, RenderTexture::Format::RGBA16F);
		m_SceneColorTex  = CreateUnique<RenderTexture>(windowWidth, windowHeight, RenderTexture::Format::RGBA8);
		m_SceneDepthTex  = CreateUnique<RenderTexture>(windowWidth, windowHeight, RenderTexture::Format::Depth24Stencil8);

		BuildRenderGraphs();
	}

	void Game::BuildRenderGraphs() {
		using namespace Rendering::RG::Passes;

		RenderTexture& gameColor{ *m_GameColorTex};
		RenderTexture& gameDepth { *m_GameDepthTex };
		RenderTexture& postColor { *m_PostProcessTex };
		RenderTexture& sceneColor { *m_SceneColorTex };
		RenderTexture& sceneDepth { *m_SceneDepthTex };

		m_GameRenderGraph.AddPass<BackgroundPass>(gameColor, gameDepth);
		m_GameRenderGraph.AddPass<OpaquePass>(gameColor, gameDepth);
		m_GameRenderGraph.AddPass<AlphaTestPass>(gameColor, gameDepth);
		m_GameRenderGraph.AddPass<TransparentPass>(gameColor, gameDepth);
		m_GameRenderGraph.AddPass<PostProcessPass>(gameColor, postColor);
		m_GameRenderGraph.Compile();

		m_SceneRenderGraph.AddPass<BackgroundPass>(sceneColor, sceneDepth);
		m_SceneRenderGraph.AddPass<OpaquePass>(sceneColor, sceneDepth);
		m_SceneRenderGraph.AddPass<AlphaTestPass>(sceneColor, sceneDepth);
		m_SceneRenderGraph.AddPass<GridPass>(sceneColor, sceneDepth);
		m_SceneRenderGraph.AddPass<TransparentPass>(sceneColor, sceneDepth);
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

		RenderSystem::Get().Render(*CameraSystem::Get().GetMainCamera(), m_GameRenderGraph);
		RenderSystem::Get().Render(*CameraSystem::Get().GetSceneCamera(), m_SceneRenderGraph);

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
			gameCam->aspect = static_cast<float>(width) / static_cast<float>(height);
		}
	}

	void Game::ResizeSceneView(const int width, const int height) {
		m_SceneColorTex->Resize(width, height);
		m_SceneDepthTex->Resize(width, height);

		m_SceneRenderGraph.Compile();

		if (auto* sceneCam = CameraSystem::Get().GetSceneCamera()) {
			sceneCam->aspect = static_cast<float>(width) / static_cast<float>(height);
		}
	}

	void Game::SetHighlightedEntity(const EntityID entityID) {
		if (entityID == Entity::ROOT_ENTITY_ID) {
			return;
		}

		auto const& entity { SceneSystem::Get().GetCurrentScene()->GetEntity(entityID) };

		if (!entity->HasComponent<Components::MeshRenderer>()) {
			return;
		}

		RenderSystem::Get().SetHighlightedEntity(entity);
	}

} // namespace Engine::Core