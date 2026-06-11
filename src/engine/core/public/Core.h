#pragma once

#include <memory>
#include <span>
#include <cstdint>

namespace Engine {

template <typename T>
using Unique = std::unique_ptr<T>;

template <typename T>
using Shared = std::shared_ptr<T>;

template <typename T>
using Weak = std::weak_ptr<T>;

using BufferView = std::span<std::byte>;
using ConstBufferView = std::span<const std::byte>;

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

} // namespace Engine