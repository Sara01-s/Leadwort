#include <Leadwort/asset-management/private/AssetKey.h>
#include <Leadwort/asset-management/public/AssetDatabase.h>

#include "Leadwort/serialization/AssetSerializer.h"

#include <Leadwort/core/public/Path.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stb_image.h>

namespace Leadwort::AssetManagement {

	using namespace Rendering::Bindables;

	AssetDatabase::AssetDatabase(std::string rootPath) : m_Root(std::move(rootPath)) {
	    LW_ASSERT(!m_Root.empty(), "AssetManager: A Root path cannot be empty.");
	}

	std::string AssetDatabase::ResolvePath(const std::string& path) const {
	    return Resolve(path).string();
	}

	std::filesystem::path AssetDatabase::Resolve(const std::string& path) const noexcept {
		return (Core::Path(m_Root) / path).GetGenericString();
	}

	std::string AssetDatabase::LoadText(const std::string& path) const {
	    const auto& fullPath { Resolve(path) };

	    std::ifstream file(fullPath, std::ios::in | std::ios::binary);
	    LW_ASSERT(file.is_open(), "AssetManager: Could not open a file at: " + fullPath.string());

	    return std::string {
	        std::istreambuf_iterator(file), std::istreambuf_iterator<char>()
	    };
	}

	std::vector<uint8_t> AssetDatabase::LoadBytes(const std::string& path) const {
	    const auto& fullPath { Resolve(path) };
	    std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
	    LW_ASSERT(file.is_open(), "AssetManager: Could not open a binary file at: " + fullPath.string());

	    const long long size = file.tellg();
	    LW_ASSERT(size > 0, "AssetManager: File is empty: " + fullPath.string());

	    file.seekg(0, std::ios::beg);
	    std::vector<uint8_t> buffer(size);
	    file.read(reinterpret_cast<char*>(buffer.data()), size);

	    return buffer;
	}


	Shared<Shader> AssetDatabase::GetShader(
		const std::string& path,
		const std::optional<std::set<std::string>>& defines
	) {
		std::string key { Resolve(path).lexically_normal().generic_string() };

		if (defines) {
			for (const auto& define : *defines) {
				key += '|';
				key += define;
			}
		}

		if (auto cached { m_ShaderCache.Get(key)}) {
			LW_LOG("AssetManager [GetShader]: cache HIT key='", key, "'");
			return cached;
		}

		LW_LOG("AssetManager [GetShader]: cache MISS key='", key, "'");
		auto shader { CreateShared<Shader>(path, defines.value_or(std::set<std::string>{}), AssetKey<Shader>{}) };
		m_ShaderCache.Set(key, shader);
		return shader;
	}

	Shared<Texture> AssetDatabase::GetTexture(const std::string_view path) {
		const std::string key { Resolve(std::string(path)).string() };

		if (auto cached { m_TextureCache.Get(key) }) {
			return cached;
		}

		if (path.ends_with(".exr")) {
			LW_LOG("AssetManager: Loading Texture: ", key);
			const auto& texture { CreateShared<Texture>(key, AssetKey<Texture>{}) };
			m_TextureCache.Set(key, texture);
			return texture;
		}

		LW_LOG("AssetManager: Loading Texture: ", key);
		const auto& bytes { LoadBytes(key) };
		const auto& texture { CreateTextureFromBytes(bytes.data(), bytes.size(), key) };

		m_TextureCache.Set(key, texture);
		return texture;
	}

	Shared<Texture> AssetDatabase::GetTextureFromAbsolutePath(const std::string& absolutePath) {
		if (auto cached = m_TextureCache.Get(absolutePath)) {
			return cached;
		}

		LW_LOG("AssetManager: Loading Texture (absolute): ", absolutePath);
		std::ifstream file(absolutePath, std::ios::binary | std::ios::ate);
		LW_ASSERT(file.is_open(), "AssetManager: Could not open the absolute texture path: " + absolutePath);

		const auto size { file.tellg() };
		file.seekg(0, std::ios::beg);
		std::vector<uint8_t> bytes(size);
		file.read(reinterpret_cast<char*>(bytes.data()), size);

		auto texture { CreateTextureFromBytes(bytes.data(), bytes.size(), absolutePath) };
		LW_LOG("AssetManager: Texture created (absolute): ", absolutePath);

		m_TextureCache.Set(absolutePath, texture);
		return texture;
	}

	Shared<Texture> AssetDatabase::GetEmbeddedTexture(const int index, const uint8_t* data, const size_t size) {
		const std::string key { "*" + std::to_string(index) };

		if (auto cached = m_TextureCache.Get(key)) {
			return cached;
		}

		LW_LOG("AssetManager: Loading embedded Texture [index=", index, "]");
		auto texture { CreateTextureFromBytes(data, size, key) }; // key = "*" + index.
		LW_ASSERT(texture != nullptr, "AssetManager: Failed to decode embedded texture at index: " + key);

		m_TextureCache.Set(key, texture);
		return texture;
	}

	Shared<CubeMap> AssetDatabase::GetCubeMap(const std::array<std::string, 6>& paths) {
		std::string key{};

		for (const auto& path : paths) {
			key += path;
			key += '|';
		}

		if (auto cached { m_CubeMapCache.Get(key) }) {
			return cached;
		}

		LW_LOG("AssetManager: Loading CubeMap: ", key);
		std::array<std::string, 6> resolvedPaths;
		for (size_t i = 0; i < 6; i++) {
			resolvedPaths[i] = Resolve(paths[i]).string();
		}

		auto cubemap { CreateShared<CubeMap>(resolvedPaths, AssetKey<CubeMap>{}) };
		LW_ASSERT(cubemap != nullptr, "AssetManager: Failed to create CubeMap.");

		m_CubeMapCache.Set(key, cubemap);
		return cubemap;
	}

	Shared<Core::Model> AssetDatabase::GetModel(const std::string& path) {
		const std::string key { Resolve(path).string() };

		if (auto cached = m_ModelCache.Get(key)) {
			return cached;
		}

		LW_LOG("AssetManager: Loading Model: ", key);
		auto model { CreateShared<Core::Model>(key, AssetKey<Core::Model>{}) };
		LW_ASSERT(model != nullptr, "AssetManager: Failed to create Model from: " + path);

		m_ModelCache.Set(key, model);
		return model;
	}

	static std::string FormatMeshKey(const MeshKey& key) {
		if (key.isCreatedAtRuntime) {
			return "[runtime] " + key.modelPath;
		}

		return "[file] " + key.modelPath + " #" + std::to_string(key.meshIndex);
	}

	Shared<Mesh> AssetDatabase::GetMesh(const MeshData& meshData) {
		if (auto cached { m_MeshCache.Get(meshData.key) }) {
			return cached;
		}

		LW_LOG("AssetManager: Loading Mesh: ", FormatMeshKey(meshData.key));
		auto mesh { CreateShared<Mesh>(meshData, AssetKey<Mesh>{}) };
		LW_ASSERT(mesh != nullptr, "AssetManager: Failed to create Mesh.");

		m_MeshCache.Set(meshData.key, mesh);
		return mesh;
	}

	std::vector<Shared<Mesh>> AssetDatabase::GetAllMeshes() const noexcept {
		return m_MeshCache.GetAllValues();
	}

	std::vector<Shared<Texture>> AssetDatabase::GetAllTextures() const noexcept {
		return m_TextureCache.GetAllValues();
	}

	Shared<Material> AssetDatabase::CreateMaterial(const Shared<Shader>& shader) {
		LW_LOG("AssetManager: Creating Material for Shader [ptr=", reinterpret_cast<uintptr_t>(shader.get()), "]");
		auto material { CreateShared<Material>(shader, AssetKey<Material>{}) };
		LW_ASSERT(material != nullptr, "AssetManager: Failed to create Material");
		return material;
	}

	Shared<Material> AssetDatabase::GetOrCreateMaterial(const MeshKey& key, const Shared<Shader>& shader) {
		if (auto cached { m_EmbeddedMaterialCache.Get(key) }) {
			return cached;
		}

		LW_LOG("AssetManager: Creating embedded Material for key='", FormatMeshKey(key), "'");
		auto material { CreateMaterial(shader) };
		m_EmbeddedMaterialCache.Set(key, material);
		return material;
	}

	Shared<Material> AssetDatabase::GetMaterial(const std::string& path) {
		const std::string key { Resolve(path).string() };

		if (auto cached { m_MaterialCache.Get(key) }) {
			return cached;
		}

		LW_LOG("AssetManager: Loading Material: ", key);
		const Json json { LoadJson(path) };
		auto material { AssetSerializer<Material>::Deserialize(json, *this) };

		m_MaterialCache.Set(key, material);
		return material;
	}

	void AssetDatabase::SaveMaterial(const std::string& path, const Material& material) const {
		const Json json { AssetSerializer<Material>::Serialize(material) };
		SaveJson(path, json);
	}

	Json AssetDatabase::LoadJson(const std::string& path) const {
		const std::string text { LoadText(path) };
		return Json::parse(text);
	}

	void AssetDatabase::SaveJson(const std::string& path, const Json& json) const {
		const auto fullPath { Resolve(path) };
		std::ofstream file(fullPath);
		LW_ASSERT(file.is_open(), "AssetManager: Could not open a file by writing: " + fullPath.string());
		file << json.dump(2); // indent = 2
	}

	void AssetDatabase::Cleanup() {
		m_ShaderCache.Cleanup();
		m_TextureCache.Cleanup();
		m_CubeMapCache.Cleanup();
		m_ModelCache.Cleanup();
		m_MeshCache.Cleanup();
		m_MaterialCache.Cleanup();
		m_EmbeddedMaterialCache.Cleanup();
	}

	Shared<Texture> AssetDatabase::CreateTextureFromBytes(const uint8_t* bytes, const size_t size, const std::string& path) {
		int width{}, height{}, channels{};
		stbi_set_flip_vertically_on_load(true);

		uint8_t* pixels { stbi_load_from_memory(
			bytes, static_cast<int>(size),
			&width, &height, &channels,
			STBI_rgb_alpha
		)};

		LW_ASSERT(pixels != nullptr, std::string("AssetManager: Texture decode failed: ") + stbi_failure_reason());

		auto texture { CreateShared<Texture>(AssetKey<Texture>{}) };
		texture->SetPath(path);
		texture->UploadRGBA(pixels, width, height, /*mips*/true, /*aniso*/true);
		stbi_image_free(pixels);

		return texture;
	}

} // namespace Engine::AssetManagement