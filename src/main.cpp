#include "editor/EditorLayer.h"
#include "engine/core/public/Game.h"
#include "engine/core/public/Window.h"
#include "engine/systems/public/RenderSystem.h"

#include <format>
#include <imgui.h>

void ShowViewport(
    const char* name,
    Engine::Rendering::Bindables::RenderTarget& renderTarget
) {
    ImGui::Begin(name);

    const ImVec2 size = ImGui::GetContentRegionAvail();
    const int newWidth  = static_cast<int>(size.x);
    const int newHeight = static_cast<int>(size.y);

    if (newWidth > 0 && newHeight > 0) {
        const bool resized = newWidth  != renderTarget.GetWidth() || newHeight != renderTarget.GetHeight();

        if (resized) {
            renderTarget.Resize(newWidth, newHeight);
        }
    	else {
            ImGui::Image(
                renderTarget.GetTextureID(),
                size,
                { 0.0f, 1.0f }, { 1.0f, 0.0f }
            );
        }
    }

    ImGui::End();
}

void ShowStatus() {
    ImGui::Begin("Status");

    ImGui::Text("GLEngine");
    ImGui::Separator();

    const float framerate = ImGui::GetIO().Framerate;
    const float frameTime = 1000.0f / framerate;

    ImGui::Text("%s", std::format("FPS: {:.1f}", framerate).c_str());
    ImGui::Text("%s", std::format("Frame Time: {:.3f} ms", frameTime).c_str());

    ImGui::End();
}

int main() {
    const Engine::Core::Game game;

    auto& window = Engine::Core::Window::Get();
    window.OnWindowResized.Subscribe([&game, &window] {
        game.Tick();
        window.SwapBuffers();
    });

    Engine::Editor::EditorLayer editor;
    editor.Init(reinterpret_cast<std::uint64_t>(window.GetHandle()));

    auto& gameRenderTarget  = game.GetGameRenderTarget();
    auto& sceneRenderTarget = game.GetSceneRenderTarget();

    Engine::Systems::RenderSystem::Get().AddOverlay([&] {
        editor.StartFrame();
        editor.SetupDockSpace();
        ShowViewport("Scene", sceneRenderTarget);
        ShowViewport("Game",  gameRenderTarget);
        ShowStatus();
        editor.EndFrame();
    });

    game.Loop();

    return 0;
}