#pragma once


#include "engine/core/public/Core.h"
#include "engine/rendering/bindables/public/CubeMap.h"
#include "engine/rendering/bindables/public/Texture.h"
#include "engine/rendering/bindables/public/Mesh.h"
#include "engine/rendering/public/Model.h"

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine::Rendering::Bindables {
	class VertexLayout;
	class Shader;
}

namespace Engine::AssetManagement {

namespace Bindables = Rendering::Bindables;

// ---------------------------------------------------------------------------
// ResourceCache
// ---------------------------------------------------------------------------

template <typename TKey, typename TValue>
class AssetCache {
public:
    [[nodiscard]] Shared<TValue> Get(const TKey& key) const {
        const auto it = m_Cache.find(key);
        if (it == m_Cache.end()) {
	        return nullptr;
        }
        return it->second.lock();
    }

    void Set(const TKey& key, const Shared<TValue>& resource) {
        m_Cache[key] = resource;
    }

    void Cleanup() {
        for (auto it = m_Cache.begin(); it != m_Cache.end();) {
            if (it->second.expired()) {
	            it = m_Cache.erase(it);
            }
            else {
	            ++it;
            }
        }
    }

private:
    std::unordered_map<TKey, Weak<TValue>> m_Cache{};
};

// ---------------------------------------------------------------------------
// AssetManager
// ---------------------------------------------------------------------------

class AssetManager {
public:
    explicit AssetManager(std::string rootPath);

    [[nodiscard]] std::string          ResolvePath(const std::string& path) const;
    [[nodiscard]] std::string          LoadText   (const std::string& path) const;
    [[nodiscard]] std::vector<uint8_t> LoadBytes  (const std::string& path) const;

    [[nodiscard]] Shared<Bindables::Shader>   GetShader                 (const std::string& path);
    [[nodiscard]] Shared<Bindables::Texture>  GetTexture                (const std::string& path);
    [[nodiscard]] Shared<Bindables::Texture>  GetTextureFromAbsolutePath(const std::string& absolutePath);
    [[nodiscard]] Shared<Bindables::Texture>  GetEmbeddedTexture        (int index, const uint8_t* data, size_t size);
    [[nodiscard]] Shared<Bindables::CubeMap>  GetCubeMap                (const std::array<std::string, 6>& paths);
    [[nodiscard]] Shared<Core::Model>         GetModel                  (const std::string& path);
	[[nodiscard]] Shared<Bindables::Mesh>	  GetMesh(const Bindables::MeshData& meshData);
	[[nodiscard]] static Shared<Bindables::Material> CreateMaterial(const Shared<Bindables::Shader>& shader);

	void Cleanup();

private:
    [[nodiscard]] static Shared<Bindables::Texture> CreateTextureFromBytes(const uint8_t* bytes, size_t size);
    [[nodiscard]] static std::string                MakeCubeMapKey        (const std::array<std::string, 6>& paths);
    [[nodiscard]] std::filesystem::path             Resolve               (const std::string& path) const;

    std::string m_Root;
    AssetCache<std::string, Bindables::Shader>      m_ShaderCache{};
    AssetCache<std::string, Bindables::Texture>     m_TextureCache{};
    AssetCache<std::string, Bindables::CubeMap>     m_CubeMapCache{};
	AssetCache<Bindables::MeshKey, Bindables::Mesh> m_MeshCache{};
    AssetCache<std::string, Core::Model>            m_ModelCache{};
};

// ---------------------------------------------------------------------------
// AssetWrapper / singletons
// ---------------------------------------------------------------------------

template <typename, const char* RootPath>
class AssetWrapper {
public:
    [[nodiscard]] static std::string          Resolve   (const std::string& path) { return Get().ResolvePath(path); }
    [[nodiscard]] static std::string          LoadText  (const std::string& path) { return Get().LoadText(path); }
    [[nodiscard]] static std::vector<uint8_t> LoadBytes (const std::string& path) { return Get().LoadBytes(path); }

    [[nodiscard]]
    static Shared<Bindables::Shader> GetShader(const std::string& path) {
        return Get().GetShader(path);
    }

    [[nodiscard]]
    static Shared<Bindables::Texture> GetTexture(const std::string& path) {
        return Get().GetTexture(path);
    }

    [[nodiscard]]
    static Shared<Bindables::Texture> GetTextureAbsolute(const std::string& path) {
        return Get().GetTextureFromAbsolutePath(path);
    }

    [[nodiscard]]
    static Shared<Bindables::Texture> GetEmbeddedTexture(const int index, const uint8_t* data, const size_t size) {
        return Get().GetEmbeddedTexture(index, data, size);
    }

	[[nodiscard]]
	static Shared<Bindables::Material> CreateMaterial(const Shared<Bindables::Shader>& shader) {
	    return Get().CreateMaterial(shader);
    }

    [[nodiscard]]
    static Shared<Bindables::CubeMap> GetCubeMap(const std::array<std::string, 6>& paths) {
        return Get().GetCubeMap(paths);
    }

    [[nodiscard]]
    static Shared<Core::Model> GetModel(const std::string& path) {
        return Get().GetModel(path);
    }

	[[nodiscard]]
	static Shared<Bindables::Mesh> GetMesh(const Bindables::MeshData& meshData) {
    	return Get().GetMesh(meshData);
    }

private:
    static AssetManager& Get() {
        static AssetManager s_Instance(RootPath);
        return s_Instance;
    }
};

inline constexpr char EngineRoot[] = ENGINE_ASSET_ROOT;
inline constexpr char GameRoot[]   = GAME_ASSET_ROOT;

struct EngineAssets : AssetWrapper<EngineAssets, EngineRoot> {};
//struct GameAssets   : AssetWrapper<GameAssets,   GameRoot>   {};

} // namespace Engine::AssetManagement