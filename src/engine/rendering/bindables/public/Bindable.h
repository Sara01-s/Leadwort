#pragma once
#include "glad/glad.h"

#include <cstdint>

namespace Engine::Rendering::Bindables {

class Bindable {
public:
	static constexpr GLuint DEFAULT_GPU_ID = 0;

	virtual ~Bindable() = default;

	virtual void Bind() const noexcept = 0;
	virtual void Unbind() const noexcept = 0;

	[[nodiscard]] GLuint GetGpuID() const noexcept { return m_GpuID; }

protected:
	Bindable() = default;

	GLuint m_GpuID{DEFAULT_GPU_ID};
};

} // namespace Core::Rendering::Bindables