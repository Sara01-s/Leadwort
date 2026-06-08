#pragma once

#include "engine/rendering/bindables/public/RenderTarget.h"
#include <memory>

namespace Engine::Core {

class Game {
	using RenderTarget = Rendering::Bindables::RenderTarget;

public:
	Game();

	void Tick() const;
	void Loop() const;

	RenderTarget& GetGameRenderTarget()  const noexcept { return *m_GameRenderTarget;  }
	RenderTarget& GetSceneRenderTarget() const noexcept { return *m_SceneRenderTarget; }

private:
	std::unique_ptr<RenderTarget> m_GameRenderTarget  = nullptr;
	std::unique_ptr<RenderTarget> m_SceneRenderTarget = nullptr;
};

} // namespace Engine::Core