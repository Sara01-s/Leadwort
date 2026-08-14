#pragma once

#include <filesystem>
#include <string_view>
#include <algorithm>
#include <cctype>

namespace Leadwort::AssetManagement {

	enum class AssetType {
		Unknown,
		Texture,
		Model,
		Shader,
		Material,
	};

	inline AssetType InferAssetType(const std::filesystem::path& path) {
		auto ext = path.extension().string();
		std::ranges::transform(ext, ext.begin(), [](const unsigned char c) { return std::tolower(c); });

		if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".exr" || ext == ".tga") {
			return AssetType::Texture;
		}

		if (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb") {
			return AssetType::Model;
		}

		if (ext == ".glsl" || ext == ".vert" || ext == ".frag" || ext == ".shader") {
			return AssetType::Shader;
		}

		if (ext == ".mat") {
			return AssetType::Material;
		}

		return AssetType::Unknown;
	}

}