#pragma once

#include <cstdint>
#include <memory>
#include <span>

namespace Leadwort {

template <typename T>
using Unique = std::unique_ptr<T>;

template <typename T>
using Shared = std::shared_ptr<T>;

template <typename T>
using Weak = std::weak_ptr<T>;

using BufferView = std::span<std::byte>;
using ConstBufferView = std::span<const std::byte>;

using GpuID = unsigned int;
using EntityID = std::uint32_t;
using Index = std::uint32_t;

template <typename T, size_t Alignment = 16>
struct alignas(Alignment) AlignedBuffer {
	T data;
};

template <typename T, typename... Args>
[[nodiscard]] Unique<T> CreateUnique(Args&&... args) {
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
[[nodiscard]] Shared<T> CreateShared(Args&&... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T>
concept HasBaseClass = requires { typename T::Base; };

template <typename>
	struct BaseOf {
	using type = void;
};

template <typename T>
requires requires { typename T::Base; }
struct BaseOf<T> {
	using type = T::Base;
};

template <typename T> using BaseOf_t = BaseOf<T>::type;

} // namespace Engine