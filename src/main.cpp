#include "editor/EditorLayer.h"
#include "engine/core/public/Game.h"
#include "engine/core/public/Window.h"
#include "engine/systems/public/RenderSystem.h"

/* TODO: Cosas para torturarte en el futuro Sara:
*   --- Must have.
*	- Arreglar el Shader Hot Reload (ShaderWatcher no hace nada el muy vago).
*	--- Nice to have.
*	- Poner las coordenadas en GUI.
*	- Poner un botón para resetear coordenadas en GUI.
*	- Añadir tests >:(.
*/

#include <format>
#include <imgui.h>

void ShowViewport(
    const char* name,
    Engine::Rendering::RenderTexture& renderTexture,
    const std::function<void(int, int)>& onResize = nullptr,
	const float targetAspect = -1.0f // 0 means free aspect ratio.
) {
    ImGui::Begin(name);

    const ImVec2 availSize = ImGui::GetContentRegionAvail();
    ImVec2 renderSize = availSize;

    if (targetAspect > 0.0f && availSize.x > 0 && availSize.y > 0) {
		const float windowAspect = availSize.x / availSize.y;

        if (windowAspect > targetAspect) {
            renderSize.y = availSize.y;
            renderSize.x = renderSize.y * targetAspect;
        } else {
            renderSize.x = availSize.x;
            renderSize.y = renderSize.x / targetAspect;
        }

		const float cursorX = (availSize.x - renderSize.x) * 0.5f;
		const float cursorY = (availSize.y - renderSize.y) * 0.5f;
        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + cursorX, ImGui::GetCursorPosY() + cursorY));
    }

    const int newWidth  = static_cast<int>(renderSize.x);
    const int newHeight = static_cast<int>(renderSize.y);

    if (newWidth > 0 && newHeight > 0) {
       const bool resized = newWidth != renderTexture.GetWidth()
                     || newHeight != renderTexture.GetHeight();

       if (resized) {
          if (onResize) {
             onResize(newWidth, newHeight);
          }
          else {
             renderTexture.Resize(newWidth, newHeight);
          }
       }

       ImGui::Image(
          renderTexture.GetGpuID(),
          renderSize,
          ImVec2(0.0f, 1.0f),
          ImVec2(1.0f, 0.0f)
       );
    }

    ImGui::End();
}

void ShowStatus() {
    ImGui::Begin("Leadwort");
    ImGui::Separator();

    ImGui::Text("Status");

    const float framerate = ImGui::GetIO().Framerate;
    const float frameTime = 1000.0f / framerate;

    ImGui::Text("%s", std::format("FPS: {:.1f}", framerate).c_str());
    ImGui::Text("%s", std::format("Frame Time: {:.3f} ms", frameTime).c_str());

    ImGui::End();
}

int main() {
    Engine::Core::Game game{};

    auto& window = Engine::Core::Window::Get();
    window.OnWindowResized.Subscribe([&game, &window] {
        game.Tick();
        window.SwapBuffers();
    });

    Engine::Editor::EditorLayer editor{};
    editor.Init(reinterpret_cast<std::uint64_t>(window.GetHandle()));

	auto& gameFrameColor = game.GetGameOutputTexture();
    auto& sceneFrameColor = game.GetSceneOutputTexture();

	Engine::Systems::RenderSystem::Get().AddOverlayCallback([&] {
		editor.StartFrame();
		editor.SetupDockSpace();

		static float chosenGameAspect = 16.0f / 9.0f;

		ImGui::Begin("Game Aspect Ratio");
		if (ImGui::BeginCombo("##", chosenGameAspect == -1.0f ? "Free" : (chosenGameAspect > 1.5f ? "16:9" : "4:3"))) {
			if (ImGui::Selectable("Free Aspect")) { chosenGameAspect = -1.0f; }
			if (ImGui::Selectable("16:9"))        { chosenGameAspect = 16.0f / 9.0f; }
			if (ImGui::Selectable("4:3"))         { chosenGameAspect = 4.0f / 3.0f; }
			ImGui::EndCombo();
		}
		ImGui::End();

		ShowViewport("Scene", sceneFrameColor, [&game](const int w, const int h) { game.ResizeSceneView(w, h); }, -1.0f);
		ShowViewport("Game", gameFrameColor, [&game](const int w, const int h) { game.ResizeGameView(w, h); }, chosenGameAspect);

		ShowStatus();
		editor.EndFrame();
	});

	game.Loop();

	return 0;
}