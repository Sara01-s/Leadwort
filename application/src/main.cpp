#include "../../engine/pch.h"

#include <Leadwort/core/public/Game.h>
#include <Leadwort/core/public/Window.h>
#include <Leadwort/systems/public/RenderSystem.h>

/* TODO: Cosas para torturarte en el futuro Sara:
 *  --- Must have.
 *	- Arreglar el Shader Hot Reload (ShaderWatcher no hace nada el muy vago).
 *	--- Nice to have.
 *	- Poner las coordenadas en GUI.
 *	- Poner un botón para resetear coordenadas en GUI.
 *	- Añadir tests >:(.
 */

#include "Leadwort/systems/public/SceneSystem.h"
#include "LeadwortEditor/windows/public/HierarchyWindow.h"

#include <LeadwortEditor/core/public/EditorCore.h>
#include <LeadwortEditor/core/public/EditorWindowsContainer.h>
#include <LeadwortEditor/windows/public/GameViewport.h>
#include <LeadwortEditor/windows/public/SceneViewport.h>
#include <LeadwortEditor/windows/public/StatusWindow.h>

int main() {
	using namespace Editor;

    Leadwort::Core::Game game{};

	const auto& window = Leadwort::Core::Window::Get();

    Core::EditorCore::Initialize(reinterpret_cast<std::uint64_t>(window.GetHandle()));

	Leadwort::Rendering::RenderTexture& gameRenderTexture { game.GetGameOutputTexture() };
    Leadwort::Rendering::RenderTexture& sceneRenderTexture { game.GetSceneOutputTexture() };

	Core::EditorWindowsContainer windowsContainer{};
	EditorContext editorContext{};

	editorContext.openedScene = Leadwort::Systems::SceneSystem::Get().GetCurrentScene();

	windowsContainer.AddWindows(
		Leadwort::CreateUnique<Windows::GameViewport>(
			&gameRenderTexture,
			[&game](const int width, const int height) { game.ResizeGameView(width, height); }
		),
		Leadwort::CreateUnique<Windows::SceneViewport>(
			&sceneRenderTexture,
			[&game](const int width, const int height) { game.ResizeSceneView(width, height); }
		),
		Leadwort::CreateUnique<Core::StatusWindow>(),
		Leadwort::CreateUnique<Windows::HierarchyWindow>(editorContext)
	);

	game.Loop([&] {
		Core::EditorCore::StartFrame();
		Core::EditorCore::SetupDockSpace();

		windowsContainer.RenderAllWindows();

		Core::EditorCore::EndFrame();
	});
}
