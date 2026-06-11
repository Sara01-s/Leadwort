#include "editor/EditorLayer.h"
#include "engine/core/public/Game.h"
#include "engine/core/public/Window.h"
#include "engine/systems/public/RenderSystem.h"

/* TODO: Cosas para torturarte en el futuro Sara:
*   --- Must have.
*	- ¿Por qué me sigue el modelo wtf?
*	- Arreglar el Shader Hot Reload (ShaderWatcher no hace nada el muy vago).
*	- Arreglar el PBR (No hay iluminación después de la diffuse).
*	- Arreglar Transform::LookAt, no mira hacia donde debe!!!!!!!!!.
*	--- Nice to have.
*	- Poner las coordenadas en GUI.
*	- Poner un botón para resetear coordenadas en GUI.
*	- Añadir tests >:(.
*/

#include <format>
#include <imgui.h>

void ShowViewport(
	const char* name,
	Engine::Rendering::Bindables::RenderTarget& renderTarget,
	const std::function<void(int, int)>& onResize = nullptr
) {
	ImGui::Begin(name);

	const ImVec2 size = ImGui::GetContentRegionAvail();
	const int newWidth  = static_cast<int>(size.x);
	const int newHeight = static_cast<int>(size.y);

	if (newWidth > 0 && newHeight > 0) {
		const bool resized = newWidth != renderTarget.GetWidth()
						  || newHeight != renderTarget.GetHeight();

		if (resized) {
			if (onResize) onResize(newWidth, newHeight);
			else          renderTarget.Resize(newWidth, newHeight);
		}

		ImGui::Image(renderTarget.GetTextureGpuID(), size, {0,1}, {1,0});
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

	auto& gameRenderTarget = game.GetGameRenderTarget();
	auto& gamePostProcessRenderTarget = game.GetGamePostProcessRenderTarget();
    auto& sceneRenderTarget = game.GetSceneRenderTarget();

    Engine::Systems::RenderSystem::Get().AddOverlay([&] {
        editor.StartFrame();
        editor.SetupDockSpace();
    	ShowViewport("Scene", sceneRenderTarget);
		ShowViewport("Game",  gamePostProcessRenderTarget, [&](const int w, const int h) { game.ResizeGameView(w, h); });
		ShowStatus();
		editor.EndFrame();
	});

	game.Loop();

	return 0;
}