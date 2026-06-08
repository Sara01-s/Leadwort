#pragma once

#include <cstdint>

namespace Engine::UI {

class UILayer {
public:
	virtual ~UILayer() = default;

	virtual void Init(std::uint64_t windowHandle) = 0;
	virtual void StartFrame() = 0;
	virtual void EndFrame() = 0;
};

} // namespace Engine::UI