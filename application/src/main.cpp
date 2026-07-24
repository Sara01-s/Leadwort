#include "../../engine/pch.h"

#include <Leadwort/core/public/Game.h>
#include <Leadwort/core/public/Window.h>
#include <Leadwort/systems/public/RenderSystem.h>

#include "Leadwort/systems/public/SceneSystem.h"
#include "LeadwortEditor/windows/public/ConsoleWindow.h"
#include "LeadwortEditor/windows/public/HierarchyWindow.h"

#include <LeadwortEditor/core/public/EditorCore.h>
#include <LeadwortEditor/core/public/EditorWindowsContainer.h>
#include <LeadwortEditor/windows/public/GameViewport.h>
#include <LeadwortEditor/windows/public/SceneViewport.h>
#include <LeadwortEditor/windows/public/StatusWindow.h>

int main() {
	using namespace Editor;

	EditorContext editorContext{};

	Leadwort::Utils::Log::LogCallback = [&editorContext](const std::string& message) {
		editorContext.logHistory.push_back(message);
		editorContext.logCallback.Execute(message);
	};

    Leadwort::Core::Game game{};

	const auto& window = Leadwort::Core::Window::Get();

    Core::EditorCore::Initialize(reinterpret_cast<std::uint64_t>(window.GetHandle()));

	Leadwort::Rendering::RenderTexture& gameRenderTexture { game.GetGameOutputTexture() };
    Leadwort::Rendering::RenderTexture& sceneRenderTexture { game.GetSceneOutputTexture() };

	Core::EditorWindowsContainer windowsContainer{};

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
		Leadwort::CreateUnique<Windows::HierarchyWindow>(editorContext),
		Leadwort::CreateUnique<Windows::ConsoleWindow>(editorContext)
	);

	game.Loop([&] {
		Core::EditorCore::StartFrame();
		Core::EditorCore::SetupDockSpace();

		game.SetHighlightedEntity(editorContext.selectedEntityID);

		windowsContainer.RenderAllWindows();

		Core::EditorCore::EndFrame();
	});
}
