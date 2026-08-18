#include "../../engine/pch.h"

#include <Leadwort/core/public/Game.h>
#include <Leadwort/core/public/Window.h>
#include <Leadwort/systems/public/RenderSystem.h>

#include "Leadwort/systems/public/SceneSystem.h"
#include "LeadwortEditor/windows/public/AssetsWindow.h"
#include "LeadwortEditor/windows/public/ConsoleWindow.h"
#include "LeadwortEditor/windows/public/HierarchyWindow.h"
#include "LeadwortEditor/windows/public/InspectorWindow.h"
#include "LeadwortEditor/windows/public/SceneTools.h"
#include "LeadwortEditor/windows/public/ToolsWindow.h"

#include <LeadwortEditor/core/public/EditorCore.h>
#include <LeadwortEditor/core/public/EditorWindowsContainer.h>
#include <LeadwortEditor/windows/public/GameViewport.h>
#include <LeadwortEditor/windows/public/SceneViewport.h>
#include <LeadwortEditor/windows/public/StatusWindow.h>

int main() {
#ifdef _WIN32
    const BOOL dpiResult { SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) };
    LW_LOG("DPI awareness set: ", (dpiResult ? "OK" : "FAILED") ," GetLastError=", GetLastError());

    const auto currentContext { GetThreadDpiAwarenessContext() };
    const auto awareness { GetAwarenessFromDpiAwarenessContext(currentContext) };
    LW_LOG("Current DPI awareness value: ", static_cast<int>(awareness));
#endif
	using namespace Editor;

	EditorContext editorContext{};

	Leadwort::Utils::Log::LogCallback = [&editorContext](const std::string& message) {
		editorContext.LogHistory.push_back(message);
		editorContext.LogCallback.Execute(message);
	};

    Leadwort::Core::Game game{};

	const auto& window { Leadwort::Core::Window::Get() };

    Core::EditorCore::Initialize(reinterpret_cast<std::uint64_t>(window.GetHandle()));

	Leadwort::Rendering::RG::RenderTexture& gameRenderTexture { game.GetGameOutputTexture() };
    Leadwort::Rendering::RG::RenderTexture& sceneRenderTexture { game.GetSceneOutputTexture() };

	Core::EditorWindowsContainer windowsContainer{};

	editorContext.OpenedScene = Leadwort::Systems::SceneSystem::Get().GetCurrentScene();

	auto gameViewportPtr { Leadwort::CreateUnique<Windows::GameViewport>(
	   &gameRenderTexture,
	   [&game](const int width, const int height) { game.ResizeGameView(width, height); }
	)};

	auto sceneViewportPtr { Leadwort::CreateUnique<Windows::SceneViewport>(
	   &sceneRenderTexture,
	   [&game](const int width, const int height) { game.ResizeSceneView(width, height); },
	   editorContext
	)};

	Windows::GameViewport* gameViewportRaw   { gameViewportPtr.get() };
	Windows::SceneViewport* sceneViewportRaw { sceneViewportPtr.get() };

	windowsContainer.AddWindows(
		std::move(gameViewportPtr),
		std::move(sceneViewportPtr),
		Leadwort::CreateUnique<Core::StatusWindow>(),
		Leadwort::CreateUnique<Windows::HierarchyWindow>(editorContext),
		Leadwort::CreateUnique<Windows::ConsoleWindow>(editorContext),
		Leadwort::CreateUnique<Windows::InspectorWindow>(editorContext),
		Leadwort::CreateUnique<Windows::AssetsWindow>(editorContext),
		Leadwort::CreateUnique<Windows::SceneTools>(editorContext),
		Leadwort::CreateUnique<Core::ToolsWindow>(editorContext)
	);

	game.Loop(
		[&] {
			gameViewportRaw->ApplyPendingResize();
			sceneViewportRaw->ApplyPendingResize();
		},
		[&] {
			Core::EditorCore::StartFrame();
			Core::EditorCore::SetupDockSpace();

			editorContext.OpenedScene = Leadwort::Systems::SceneSystem::Get().GetCurrentScene();

			if (const auto* selectedID { std::get_if<Leadwort::EntityID>(&editorContext.Selection)}) {
				game.SetHighlightedEntity(*selectedID);
			}
			else {
				game.SetHighlightedEntity(Leadwort::Core::Entity::ROOT_ENTITY_ID);
			}

			windowsContainer.RenderAllWindows();

			Core::EditorCore::EndFrame();
		}
	);

	Leadwort::Utils::Log::LogCallback = nullptr;
}

