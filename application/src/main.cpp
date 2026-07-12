#include "../../engine/pch.h"

#include "core/public/Game.h"
#include "core/public/Window.h"
#include "systems/public/RenderSystem.h"

/* TODO: Cosas para torturarte en el futuro Sara:
 *  --- Must have.
 *	- Arreglar el Shader Hot Reload (ShaderWatcher no hace nada el muy vago).
 *	- Cambiar Engine namespace a Leadwort.
 *	- Cambiar el src include de Leadwort a LeadwortEngine
 *	--- Nice to have.
 *	- Poner las coordenadas en GUI.
 *	- Poner un botón para resetear coordenadas en GUI.
 *	- Añadir tests >:(.
 */

#include <LeadwortEditor/core/public/EditorCore.h>
#include <LeadwortEditor/core/public/EditorWindowsContainer.h>
#include <LeadwortEditor/windows/public/GameViewport.h>
#include <LeadwortEditor/windows/public/SceneViewport.h>
#include <LeadwortEditor/windows/public/StatusWindow.h>

int main() {
	using namespace Editor;

    Engine::Core::Game game{};

    auto& window = Engine::Core::Window::Get();
    window.OnWindowResized.Subscribe([&game, &window] {
        game.Tick();
        window.SwapBuffers();
    });

    Core::EditorCore::Initialize(reinterpret_cast<std::uint64_t>(window.GetHandle()));

	Engine::Rendering::RenderTexture& gameRenderTexture = game.GetGameOutputTexture();
    Engine::Rendering::RenderTexture& sceneRenderTexture = game.GetSceneOutputTexture();

	Core::EditorWindowsContainer windowsContainer{};

	windowsContainer.AddWindows(
		Engine::CreateUnique<Windows::GameViewport>(
			&gameRenderTexture,
			[&game](const int width, const int height) { game.ResizeGameView(width, height); }
		),
		Engine::CreateUnique<Windows::SceneViewport>(
			&sceneRenderTexture,
			[&game](const int width, const int height) { game.ResizeSceneView(width, height); }
		),
		Engine::CreateUnique<Core::StatusWindow>()
	);

	game.Loop([&] {
		Core::EditorCore::StartFrame();
		Core::EditorCore::SetupDockSpace();

		windowsContainer.RenderAllWindows();

		Core::EditorCore::EndFrame();
	});


	return 0;
}
