#pragma once

#include "Core.h"
#include "engine/rendering/bindables/public/RenderTarget.h"
#include <memory>

namespace Engine::Core {

class Game {
	using RenderTarget = Rendering::Bindables::RenderTarget;

public:
	Game();

	void Tick() const;
	void Loop() const;

	void ResizeGameView(int width, int height) const;

	RenderTarget& GetGameRenderTarget() const noexcept { return *m_GameRenderTarget;  }
	RenderTarget& GetGamePostProcessRenderTarget() const noexcept { return *m_GamePostProcessRenderTarget; }
	RenderTarget& GetSceneRenderTarget() const noexcept { return *m_SceneRenderTarget; }

private:
	Unique<RenderTarget> m_GameRenderTarget  = nullptr;
	Unique<RenderTarget> m_GamePostProcessRenderTarget = nullptr;
	Unique<RenderTarget> m_SceneRenderTarget = nullptr;
};

} // namespace Engine::Core