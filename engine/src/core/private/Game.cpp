#include "core/public/Game.h"

#include "asset-management/public/AssetManager.h"
#include "core/public/Time.h"
#include "core/public/Window.h"
#include "rendering/public/DefaultRenderPasses.h"
#include "systems/public/BehaviourSystem.h"
#include "systems/public/CameraSystem.h"
#include "systems/public/RenderSystem.h"
#include "systems/public/Input.h"
#include "systems/public/SceneSystem.h"

#include <GLFW/glfw3.h>

namespace Engine::Core {

using namespace Rendering;
using namespace Systems;

Game::Game() {
	if (!Window::Get().Initialize(1920, 1080, "Leadwort")) {
		CORE_ERROR("Error to initialize Engine");
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
	using namespace Rendering::Passes;

	auto* gameColor { m_GameColorTex.get() };
	auto* gameDepth { m_GameDepthTex.get() };
	auto* postColor { m_PostProcessTex.get() };
	auto* sceneColor { m_SceneColorTex.get() };
	auto* sceneDepth { m_SceneDepthTex.get() };

	m_GameRenderGraph.AddPass(CreateUnique<BackgroundPass>(gameColor, gameDepth));
	m_GameRenderGraph.AddPass(CreateUnique<OpaquePass>(gameColor, gameDepth));
	m_GameRenderGraph.AddPass(CreateUnique<AlphaTestPass>(gameColor, gameDepth));
	m_GameRenderGraph.AddPass(CreateUnique<TransparentPass>(gameColor, gameDepth));
	m_GameRenderGraph.AddPass(CreateUnique<PostProcessPass>(gameColor, postColor));
	m_GameRenderGraph.Compile();

	m_SceneRenderGraph.AddPass(CreateUnique<BackgroundPass>(sceneColor, sceneDepth));
	m_SceneRenderGraph.AddPass(CreateUnique<OpaquePass>(sceneColor, sceneDepth));
	m_SceneRenderGraph.AddPass(CreateUnique<AlphaTestPass>(sceneColor, sceneDepth));
	m_SceneRenderGraph.AddPass(CreateUnique<GridPass>(sceneColor, sceneDepth));
	m_SceneRenderGraph.AddPass(CreateUnique<TransparentPass>(sceneColor, sceneDepth));
	m_SceneRenderGraph.Compile();
}

void Game::Tick() const {
    SceneSystem::Get().LoadPendingScene();

    Time::Update(glfwGetTime());
    Input::Update(Time::GetDeltaTime());

    while (Time::ShouldRunFixedUpdate()) {
        BehaviourSystem::Get().FixedUpdate();
        Time::ConsumeFixedUpdate();
    }

    BehaviourSystem::Get().Update();

	RenderSystem::Get().Render(*CameraSystem::Get().GetMainCamera(),        m_GameRenderGraph);
	RenderSystem::Get().Render(*CameraSystem::Get().GetSceneCamera(), m_SceneRenderGraph);

	RenderSystem::Get().ClearScreen();
    RenderSystem::Get().RenderUI();
}

void Game::Loop() const {
	Utils::Log::Header("Hello World!");

    while (Window::Get().IsOpen()) {
        Window::PollEvents();
        Tick();
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

} // namespace Engine::Core