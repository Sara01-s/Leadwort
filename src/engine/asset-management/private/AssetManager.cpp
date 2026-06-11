#include "engine/asset-management/public/AssetManager.h"
#include "engine/asset-management/private/AssetKey.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stb_image.h>

namespace Engine::AssetManagement {

using namespace Rendering::Bindables;

AssetManager::AssetManager(std::string rootPath) : m_Root(std::move(rootPath)) {
    CORE_ASSERT(!m_Root.empty(), "AssetManager: Root path cannot be empty.");
}

std::string AssetManager::ResolvePath(const std::string& path) const {
    return Resolve(path).string();
}

std::string AssetManager::LoadText(const std::string& path) const {
    const auto fullPath = Resolve(path);
    std::ifstream file(fullPath, std::ios::in | std::ios::binary);
    CORE_ASSERT(file.is_open(), "AssetManager: Could not open file at: " + fullPath.string());

    return {
        std::istreambuf_iterator(file), std::istreambuf_iterator<char>()
    };
}

std::vector<uint8_t> AssetManager::LoadBytes(const std::string& path) const {
    const auto fullPath = Resolve(path);
    std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
    CORE_ASSERT(file.is_open(), "AssetManager: Could not open binary file at: " + fullPath.string());

    const long long size = file.tellg();
    CORE_ASSERT(size > 0, "AssetManager: File is empty: " + fullPath.string());

    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    return buffer;
}


Shared<Shader> AssetManager::GetShader(const std::string& path) {
	const std::string key = Resolve(path).string();

	if (auto cached = m_ShaderCache.Get(key)) {
		return cached;
	}

	CORE_LOG("AssetManager: Loading Shader: ", key);
	auto shader = CreateShared<Shader>(path, AssetKey<Shader>{});
	CORE_ASSERT(shader != nullptr, "AssetManager: Failed to create Shader from: " + path);

	m_ShaderCache.Set(key, shader);
	return shader;
}

Shared<Texture> AssetManager::GetTexture(const std::string& path) {
	const std::string key = Resolve(path).string();

	if (auto cached = m_TextureCache.Get(key)) {
		return cached;
	}

	CORE_LOG("AssetManager: Loading Texture: ", key);
	const auto bytes = LoadBytes(path);
	auto texture = CreateTextureFromBytes(bytes.data(), bytes.size());

	m_TextureCache.Set(key, texture);
	return texture;
}

Shared<Texture> AssetManager::GetTextureFromAbsolutePath(const std::string& absolutePath) {
	if (auto cached = m_TextureCache.Get(absolutePath)) {
		return cached;
	}

	CORE_LOG("AssetManager: Loading Texture (absolute): ", absolutePath);
	std::ifstream file(absolutePath, std::ios::binary | std::ios::ate);
	CORE_ASSERT(file.is_open(), "AssetManager: Could not open absolute texture path: " + absolutePath);

	const auto size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<uint8_t> bytes(size);
	file.read(reinterpret_cast<char*>(bytes.data()), size);

	auto texture = CreateTextureFromBytes(bytes.data(), bytes.size());
	CORE_LOG("AssetManager: Texture created (absolute): ", absolutePath);

	m_TextureCache.Set(absolutePath, texture);
	return texture;
}

Shared<Texture> AssetManager::GetEmbeddedTexture(const int index, const uint8_t* data, const size_t size) {
	const std::string key = "*" + std::to_string(index);

	if (auto cached = m_TextureCache.Get(key)) {
		return cached;
	}

	CORE_LOG("AssetManager: Loading embedded Texture [index=", index, "]");
	auto texture = CreateTextureFromBytes(data, size);
	CORE_ASSERT(texture != nullptr, "AssetManager: Failed to decode embedded texture at index: " + key);

	m_TextureCache.Set(key, texture);
	return texture;
}

Shared<CubeMap> AssetManager::GetCubeMap(const std::array<std::string, 6>& paths) {
	const std::string key = MakeCubeMapKey(paths);

	if (auto cached = m_CubeMapCache.Get(key)) { return cached; }

	CORE_LOG("AssetManager: Loading CubeMap: ", key);
	std::array<std::string, 6> resolvedPaths;
	for (size_t i = 0; i < 6; i++) {
		resolvedPaths[i] = Resolve(paths[i]).string();
	}

	auto cubemap = CreateShared<CubeMap>(resolvedPaths, AssetKey<CubeMap>{});
	CORE_ASSERT(cubemap != nullptr, "AssetManager: Failed to create CubeMap.");

	m_CubeMapCache.Set(key, cubemap);
	return cubemap;
}

Shared<Core::Model> AssetManager::GetModel(const std::string& path) {
	const std::string key = Resolve(path).string();

	if (auto cached = m_ModelCache.Get(key)) {
		return cached;
	}

	CORE_LOG("AssetManager: Loading Model: ", key);
	auto model = CreateShared<Core::Model>(key, AssetKey<Core::Model>{});
	CORE_ASSERT(model != nullptr, "AssetManager: Failed to create Model from: " + path);

	m_ModelCache.Set(key, model);
	return model;
}

Shared<Material> AssetManager::CreateMaterial(const Shared<Shader>& shader) {
	CORE_LOG("AssetManager: Creating Material for Shader [ptr=", reinterpret_cast<uintptr_t>(shader.get()), "]");
	auto material = CreateShared<Material>(shader, AssetKey<Material>{});
	CORE_ASSERT(material != nullptr, "AssetManager: Failed to create Material");

	return material;
}

static std::string FormatMeshKey(const MeshKey& key) {
	if (key.isCreatedAtRuntime) {
		return "[runtime] " + key.modelPath;
	}

	return "[file] " + key.modelPath + " #" + std::to_string(key.meshIndex);
}

Shared<Mesh> AssetManager::GetMesh(const MeshData& meshData) {
	if (auto cached = m_MeshCache.Get(meshData.key)) {
		return cached;
	}

	CORE_LOG("AssetManager: Loading Mesh: ", FormatMeshKey(meshData.key));
	auto mesh = CreateShared<Mesh>(meshData, AssetKey<Mesh>{});
	CORE_ASSERT(mesh != nullptr, "AssetManager: Failed to create Mesh.");

	m_MeshCache.Set(meshData.key, mesh);
	return mesh;
}

void AssetManager::Cleanup() {
    m_ShaderCache.Cleanup();
    m_TextureCache.Cleanup();
    m_CubeMapCache.Cleanup();
    m_ModelCache.Cleanup();
	m_MeshCache.Cleanup();
}

Shared<Texture> AssetManager::CreateTextureFromBytes(const uint8_t* bytes, const size_t size) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);

    uint8_t* pixels = stbi_load_from_memory(
        bytes, static_cast<int>(size),
        &width, &height, &channels,
        STBI_rgb_alpha
    );

    CORE_ASSERT(pixels != nullptr, std::string("AssetManager: Texture decode failed: ") + stbi_failure_reason());

    auto texture = CreateShared<Texture>(AssetKey<Texture>{});
    texture->UploadRGBA(pixels, width, height, /*mips*/true, /*aniso*/true);
    stbi_image_free(pixels);

    return texture;
}

std::string AssetManager::MakeCubeMapKey(const std::array<std::string, 6>& paths) {
    std::string key;
    for (const auto& p : paths) {
	    key += p; key += '|';
    }
    return key;
}

std::filesystem::path AssetManager::Resolve(const std::string& path) const {
	std::string p = path;
	std::ranges::replace(p, '\\', '/');

	if (!p.empty() && p[0] == '/') { p.erase(0, 1); }

	return std::filesystem::path(m_Root) / p;
}

} // namespace Engine::AssetManagement