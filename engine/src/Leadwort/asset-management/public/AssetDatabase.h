#pragma once


#include <Leadwort/core/public/Core.h>
#include <Leadwort/rendering/bindables/public/CubeMap.h>
#include <Leadwort/rendering/bindables/public/Texture.h>
#include <Leadwort/rendering/bindables/public/Mesh.h>
#include <Leadwort/rendering/public/Model.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace Leadwort::Rendering::Bindables {
	class VertexLayout;
	class Shader;
}

namespace Leadwort::AssetManagement {

	namespace Bindables = Rendering::Bindables;

	// ResourceCache
	template <typename TKey, typename TValue>
	class AssetCache {
	public:
	    [[nodiscard]] Shared<TValue> Get(const TKey& key) const noexcept {
	        const auto it = m_Cache.find(key);

	        if (it == m_Cache.end()) {
		        return nullptr;
	        }

	        return it->second.lock();
	    }

	    void Set(const TKey& key, const Shared<TValue>& resource) noexcept {
	        m_Cache[key] = resource;
	    }

	    void Cleanup() noexcept {
	        for (auto it = m_Cache.begin(); it != m_Cache.end();) {
	            if (it->second.expired()) {
		            it = m_Cache.erase(it);
	            }
	            else {
		            ++it;
	            }
	        }
	    }

		std::vector<Shared<TKey>> GetAllKeys() const noexcept {
    		std::vector<Shared<TKey>> result{};

    		for (const auto& pair : m_Cache) {
    			result.push_back(pair.first);
    		}

    		return result;
		}

		std::vector<Shared<TValue>> GetAllValues() const noexcept {
    		std::vector<Shared<TValue>> result{};

    		for (const auto& pair : m_Cache) {
    			result.push_back(pair.second.lock());
    		}

    		return result;
		}

	private:
	    std::unordered_map<TKey, Weak<TValue>> m_Cache{};
	};

	class AssetDatabase {
	public:
	    explicit AssetDatabase(std::string rootPath);

	    [[nodiscard]] std::string          ResolvePath(const std::string& path) const;
	    [[nodiscard]] std::string          LoadText   (const std::string& path) const;
	    [[nodiscard]] std::vector<uint8_t> LoadBytes  (const std::string& path) const;

	    [[nodiscard]] Shared<Bindables::Shader>   GetShader(const std::string& path,
														    const std::optional<std::set<std::string>>& defines);
	    [[nodiscard]] Shared<Bindables::Texture>  GetTexture                (std::string_view path);
	    [[nodiscard]] Shared<Bindables::Texture>  GetTextureFromAbsolutePath(const std::string& absolutePath);
	    [[nodiscard]] Shared<Bindables::Texture>  GetEmbeddedTexture        (int index, const uint8_t* data, size_t size);
	    [[nodiscard]] Shared<Bindables::CubeMap>  GetCubeMap                (const std::array<std::string, 6>& paths);
	    [[nodiscard]] Shared<Core::Model>         GetModel                  (const std::string& path);
		[[nodiscard]] Shared<Bindables::Mesh>	  GetMesh(const Bindables::MeshData& meshData);
		[[nodiscard]] Shared<Bindables::Material> GetOrCreateMaterial(const Bindables::MeshKey& key, const Shared<Bindables::Shader>& shader);
		[[nodiscard]] static Shared<Bindables::Material> CreateMaterial(const Shared<Bindables::Shader>& shader);
		[[nodiscard]] const std::string& GetRootPath() const noexcept { return m_Root; }

		[[nodiscard]] std::vector<Shared<Bindables::Mesh>> GetAllMeshes() const noexcept;
		[[nodiscard]] std::vector<Shared<Bindables::Texture>> GetAllTextures() const noexcept;

		[[nodiscard]] Shared<Bindables::Material> GetMaterial(const std::string& path);
		void SaveMaterial(const std::string& path, const Bindables::Material& material) const;

		[[nodiscard]] Json LoadJson(const std::string& path) const;
		void SaveJson(const std::string& path, const Json& json) const;

		void Cleanup();

	private:
		[[nodiscard]] static Shared<Bindables::Texture> CreateTextureFromBytes(const uint8_t* bytes, size_t size, const std::string& path);
	    [[nodiscard]] std::filesystem::path Resolve (const std::string& path) const noexcept;

	    std::string m_Root{};
	    AssetCache<std::string, Bindables::Shader>      m_ShaderCache{};
	    AssetCache<std::string, Bindables::Texture>     m_TextureCache{};
	    AssetCache<std::string, Bindables::CubeMap>     m_CubeMapCache{};
		AssetCache<Bindables::MeshKey, Bindables::Mesh> m_MeshCache{};
	    AssetCache<std::string, Core::Model>            m_ModelCache{};
		AssetCache<std::string, Bindables::Material>    m_MaterialCache{};
		AssetCache<Bindables::MeshKey, Bindables::Material> m_EmbeddedMaterialCache{};
	};

	template <typename, const char* RootPath>
	class AssetWrapper {
	public:
	    [[nodiscard]] static std::string          ResolvePath (const std::string& path) { return Get().ResolvePath(path); }
	    [[nodiscard]] static std::string          LoadText    (const std::string& path) { return Get().LoadText(path); }
	    [[nodiscard]] static std::vector<uint8_t> LoadBytes   (const std::string& path) { return Get().LoadBytes(path); }
		[[nodiscard]] static const std::string& GetRootPath() { return Get().GetRootPath(); }

	    [[nodiscard]]
	    static Shared<Bindables::Shader> GetShader(
    		const std::string& path,
    		const std::optional<std::set<std::string>>& defines = std::nullopt
	    ) {
	        return Get().GetShader(path, defines);
	    }

	    [[nodiscard]]
	    static Shared<Bindables::Texture> GetTexture(const std::string_view path) {
	        return Get().GetTexture(path);
	    }

	    [[nodiscard]]
	    static Shared<Bindables::Texture> GetTextureFromAbsolutePath(const std::string& path) {
	        return Get().GetTextureFromAbsolutePath(path);
	    }

		[[nodiscard]]
		static std::vector<Shared<Bindables::Texture>> GetAllTextures() {
	    	return Get().GetAllTextures();
	    }

	    [[nodiscard]]
	    static Shared<Bindables::Texture> GetEmbeddedTexture(const int index, const uint8_t* data, const size_t size) {
	        return Get().GetEmbeddedTexture(index, data, size);
	    }

		[[nodiscard]]
		static Shared<Bindables::Material> GetOrCreateMaterial(const Bindables::MeshKey& key, const Shared<Bindables::Shader>& shader) {
		    return Get().GetOrCreateMaterial(key, shader);
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

		[[nodiscard]]
		static std::vector<Shared<Bindables::Mesh>> GetAllMeshes() {
    		return Get().GetAllMeshes();
	    }

		[[nodiscard]]
		static Shared<Bindables::Material> CreateMaterial(const Shared<Bindables::Shader>& shader) {
	    	return Get().CreateMaterial(shader);
	    }

		[[nodiscard]]
		static Shared<Bindables::Material> GetMaterial(const std::string& path) {
	    	return Get().GetMaterial(path);
	    }

	private:
	    static AssetDatabase& Get() {
	        static AssetDatabase s_Instance(RootPath);
	        return s_Instance;
	    }
	};

	inline constexpr char EngineRoot[] = ENGINE_ASSET_ROOT;
	inline constexpr char GameRoot[]   = GAME_ASSET_ROOT;

	struct EngineAssets : AssetWrapper<EngineAssets, EngineRoot> {};
	struct GameAssets   : AssetWrapper<GameAssets,   GameRoot>   {};

} // namespace Engine::AssetManagement