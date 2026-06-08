#include "engine/core/public/Game.h"

#include "engine/core/public/Time.h"
#include "engine/core/public/Window.h"
#include "engine/asset-management/public/AssetManager.h"
#include "engine/systems/public/BehaviourSystem.h"
#include "engine/systems/public/CameraSystem.h"
#include "engine/systems/public/Input.h"
#include "engine/systems/public/RenderSystem.h"
#include "engine/systems/public/SceneSystem.h"
#include "engine/systems/public/ShaderWatcher.h"

#include <GLFW/glfw3.h>

using Engine::Systems::BehaviourSystem;
using Engine::Systems::CameraSystem;
using Engine::Systems::Input;
using Engine::Systems::Key;
using Engine::Systems::RenderSystem;
using Engine::Systems::SceneSystem;
using Engine::Systems::ShaderWatcher;

namespace Engine::Core {

Game::Game() {
    Window::Get().Initialize(1280, 720, "GLEngine uwu");
    Utils::Log::Initialize();
    RenderSystem::Get().Init();
    Input::Init(Window::Get().GetHandle());

    SceneSystem::Get().LoadEmptyScene();
    SceneSystem::Get().LoadPendingScene();

    m_GameRenderTarget  = std::make_unique<RenderTarget>(Window::Get().GetWidth(), Window::Get().GetHeight());
    m_SceneRenderTarget = std::make_unique<RenderTarget>(Window::Get().GetWidth(), Window::Get().GetHeight());
}

void Game::Tick() const {
    if (Input::Keyboard::IsJustPressed(Key::F5)) {
        for (auto const& entry : ShaderWatcher::Get().GetActiveShaderEntries()) {
            entry.shader->Compile();
        }
    }

    SceneSystem::Get().LoadPendingScene();

    Time::Update(glfwGetTime());
    Input::Update(Time::GetDeltaTime());

    while (Time::ShouldRunFixedUpdate()) {
        BehaviourSystem::Get().FixedUpdate();
        Time::ConsumeFixedUpdate();
    }

    BehaviourSystem::Get().Update();

    RenderSystem::Get().Render(CameraSystem::Get().GetMain(), m_GameRenderTarget.get());
    RenderSystem::Get().Render(CameraSystem::Get().GetSceneCamera(), m_SceneRenderTarget.get());

    RenderSystem::Get().RenderUI();
}

void Game::Loop() const {
    CORE_LOG(AssetManagement::EngineAssets::LoadText("test.txt"));

    while (Window::Get().IsOpen()) {
        Window::PollEvents();
        Tick();
        Window::Get().SwapBuffers();
    }
}

} // namespace Engine::Core