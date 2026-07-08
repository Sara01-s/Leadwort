#pragma once
#include "core/public/Core.h"

namespace Engine::Rendering::Bindables {

class Bindable {
public:
	static constexpr GpuID DEFAULT_GPU_ID { 0 };

	virtual ~Bindable() = default;

	virtual void Bind() const noexcept = 0;
	virtual void Unbind() const noexcept = 0;

	[[nodiscard]] GpuID GetGpuID() const noexcept { return m_GpuID; }

protected:
	Bindable() = default;

	GpuID m_GpuID{ DEFAULT_GPU_ID };
};

} // namespace Core::Rendering::Bindables