#include "engine/core/public/Game.h"

#include "engine/core/public/Time.h"
#include "engine/core/public/Window.h"
#include "engine/asset-management/public/AssetManager.h"
#include "engine/systems/public/BehaviourSystem.h"
#include "engine/systems/public/CameraSystem.h"
#include "engine/systems/public/Input.h"
#include "engine/systems/public/RenderSystem.h"
#include "engine/systems/public/SceneSystem.h"

#include <GLFW/glfw3.h>

using Engine::Systems::BehaviourSystem;
using Engine::Systems::CameraSystem;
using Engine::Systems::Input;
using Engine::Systems::Key;
using Engine::Systems::RenderSystem;
using Engine::Systems::SceneSystem;

namespace Engine::Core {

Game::Game() {
	if (!Window::Get().Initialize(1280, 720, "Leadwort")) {
		CORE_ERROR("Error to initialize Engine");
	}

    Utils::Log::Initialize();
    RenderSystem::Get().Initialize();
    Input::Init(Window::Get().GetHandle());

    SceneSystem::Get().LoadEmptyScene();
    SceneSystem::Get().LoadPendingScene();

	const int windowWidth = Window::Get().GetWidth();
	const int windowHeight = Window::Get().GetHeight();

    m_GameRenderTarget  = CreateUnique<RenderTarget>(windowWidth, windowHeight);
	m_GamePostProcessRenderTarget = CreateUnique<RenderTarget>(windowWidth, windowHeight);
    m_SceneRenderTarget = CreateUnique<RenderTarget>(windowWidth, windowHeight);
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

 //    RenderSystem::Get().Render(CameraSystem::Get().GetMain(), m_GameRenderTarget.get());
	// RenderSystem::Get().RenderPostProcess(m_GameRenderTarget.get(), m_GamePostProcessRenderTarget.get());

    RenderSystem::Get().Render(CameraSystem::Get().GetSceneCamera(), m_SceneRenderTarget.get());

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

void Game::ResizeGameView(const int width, const int height) const {
	m_GameRenderTarget->Resize(width, height);
	m_GamePostProcessRenderTarget->Resize(width, height);
}

} // namespace Engine::Core