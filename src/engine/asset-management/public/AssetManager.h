#pragma once

#include "engine/rendering/bindables/public/CubeMap.h"
#include "engine/rendering/bindables/public/Shader.h"
#include "engine/rendering/bindables/public/Texture.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine::AssetManagement {

class AssetManager {
public:
    explicit AssetManager(std::string rootPath) : m_Root(std::move(rootPath)) {}

	[[nodiscard]] std::string ResolvePath(const std::string& path) const {
	    return Resolve(path).string();
    }

    [[nodiscard]] std::string LoadText(const std::string& path) const {
        const auto fullPath = Resolve(path);
        std::ifstream file(fullPath, std::ios::in | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("File not found: " + fullPath.string());
        }

        return { std::istreambuf_iterator(file), std::istreambuf_iterator<char>() };
    }

    [[nodiscard]] std::vector<uint8_t> LoadBytes(const std::string& path) const {
        const auto fullPath = Resolve(path);
        std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("File not found: " + fullPath.string());
        }
        const long long size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<uint8_t> buffer(size);
        file.read(reinterpret_cast<char*>(buffer.data()), size);
        return buffer;
    }

    [[nodiscard]] std::shared_ptr<Rendering::Bindables::Shader> LoadShader(const std::string& path) {
        const std::string key = Resolve(path).string();

        if (auto shared = m_ShaderCache[key].lock()) {
            return shared;
        }

        auto shader = std::make_shared<Rendering::Bindables::Shader>(LoadText(path), path);
        m_ShaderCache[key] = shader;
        return shader;
    }

    [[nodiscard]] std::shared_ptr<Rendering::Bindables::Texture> LoadTexture(const std::string& path) {
        const std::string key = Resolve(path).string();

        if (auto shared = m_TextureCache[key].lock()) {
            return shared;
        }

        const auto bytes = LoadBytes(path);
        auto texture = std::shared_ptr<Rendering::Bindables::Texture>(
            Rendering::Bindables::Texture::FromMemory(bytes.data(), bytes.size())
        );
        m_TextureCache[key] = texture;
        return texture;
    }

    [[nodiscard]] std::shared_ptr<Rendering::Bindables::CubeMap> LoadCubeMap(const std::array<std::string, 6>& paths) const {
        std::array<std::string, 6> resolvedPaths;
        for (size_t i = 0; i < 6; ++i) {
            resolvedPaths[i] = Resolve(paths[i]).string();
        }
        return std::make_shared<Rendering::Bindables::CubeMap>(resolvedPaths);
    }

    // Periodically removes expired weak_ptrs from the maps to keep them clean.
    void Cleanup() {
        for (auto it = m_ShaderCache.begin(); it != m_ShaderCache.end();) {
            if (it->second.expired()) {
				it = m_ShaderCache.erase(it);
			}
			else {
				++it;
			}
		}
        for (auto it = m_TextureCache.begin(); it != m_TextureCache.end();) {
            if (it->second.expired()) {
				it = m_TextureCache.erase(it);
			}
			else {
				++it;
			}
		}
    }

private:
    [[nodiscard]] std::filesystem::path Resolve(const std::string& path) const {
        std::string p = path;
        std::ranges::replace(p, '\\', '/');

        if (!p.empty() && p[0] == '/') {
            p.erase(0, 1);
        }

        return std::filesystem::path(m_Root) / p;
    }

    std::string m_Root;
    // Caches use weak_ptr to allow automatic destruction of resources.
    std::unordered_map<std::string, std::weak_ptr<Rendering::Bindables::Shader>>  m_ShaderCache;
    std::unordered_map<std::string, std::weak_ptr<Rendering::Bindables::Texture>> m_TextureCache;
};

template <typename, const char* RootPath>
class AssetWrapper {
public:
	static std::string			Resolve   (const std::string& path) { return Get().ResolvePath(path); }
	static std::string          LoadText  (const std::string& path) { return Get().LoadText(path); }
    static std::vector<uint8_t> LoadBytes (const std::string& path) { return Get().LoadBytes(path); }

    static std::shared_ptr<Rendering::Bindables::Shader>  LoadShader (const std::string& path)                 { return Get().LoadShader(path); }
    static std::shared_ptr<Rendering::Bindables::Texture> LoadTexture(const std::string& path)                 { return Get().LoadTexture(path); }
    static std::shared_ptr<Rendering::Bindables::CubeMap> LoadCubeMap(const std::array<std::string, 6>& paths) { return Get().LoadCubeMap(paths); }

private:
    static AssetManager& Get() {
        static AssetManager s_Instance(RootPath);
        return s_Instance;
    }
};

inline constexpr char EngineRoot[] = ENGINE_ASSET_ROOT;
inline constexpr char GameRoot[]   = GAME_ASSET_ROOT;

struct EngineAssets : AssetWrapper<EngineAssets, EngineRoot> {};
struct GameAssets   : AssetWrapper<GameAssets,   GameRoot>   {};

} // namespace Engine::AssetManagement