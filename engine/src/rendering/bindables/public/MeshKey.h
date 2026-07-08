#pragma once

#include <cstdint>
#include <functional>
#include <glad/glad.h>
#include <memory>
#include <vector>

namespace Engine::Rendering::Bindables {
struct MeshKey {
	std::string modelPath;
	std::uint32_t meshIndex;
	bool isCreatedAtRuntime = false;

	static MeshKey CreateAtRuntime(const std::string& name) {
		return MeshKey { "runtime://" + name, 0, true };
	}

	bool operator==(const MeshKey& other) const { return meshIndex == other.meshIndex && modelPath == other.modelPath; }
	bool operator!=(const MeshKey& other) const { return !(*this == other); }
};
}

template <>
struct std::hash<Engine::Rendering::Bindables::MeshKey> {
	size_t operator()(const Engine::Rendering::Bindables::MeshKey& key) const noexcept {
		const size_t h1 = std::hash<std::string>{}(key.modelPath);
		const size_t h2 = std::hash<std::uint32_t>{}(key.meshIndex);

		return h1 ^ h2 << 1;
	}
};