#pragma once
#include <Leadwort/core/public/Core.h>

namespace Leadwort::Rendering::Bindables {

	class IBindable {
	public:
		static constexpr GpuID DEFAULT_GPU_ID { 0 };

		virtual ~IBindable() = default;

		IBindable(const IBindable&) = delete;
		IBindable& operator=(const IBindable&) = delete;

		virtual void Bind() const noexcept = 0;
		virtual void Unbind() const noexcept = 0;

		[[nodiscard]] GpuID GetGpuID() const noexcept { return m_GpuID; }

	protected:
		IBindable() = default;

		GpuID m_GpuID { DEFAULT_GPU_ID };
	};

} // namespace Core::Rendering::Bindables