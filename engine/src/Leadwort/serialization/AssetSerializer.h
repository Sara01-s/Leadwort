#pragma once
#include "../asset-management/public/AssetDatabase.h"
#include "../asset-management/public/AssetTypes.h"
#include "Leadwort/core/public/Core.h"
#include "Leadwort/serialization/ISerializable.h"

namespace Leadwort::Rendering::Bindables {
class Material;
}
namespace Leadwort::AssetManagement {

template <typename T>
struct AssetSerializer {};

template <>
struct AssetSerializer<Bindables::Material> {
    static Shared<Bindables::Material> Deserialize(const Json& json, AssetDatabase& db) {
        const std::string shaderPath { json.at("shader").get<std::string>() };

        std::optional<std::set<std::string>> defines{};
        if (json.contains("shaderDefines")) {
            std::set<std::string> definesSet{};
            for (const auto& define : json.at("shaderDefines")) {
                definesSet.insert(define.get<std::string>());
            }
            defines = std::move(definesSet);
        }

        const auto shader { db.GetShader(shaderPath, defines) };
        LW_ASSERT(shader != nullptr, "AssetSerializer<Material>: Failed to resolve shader: " + shaderPath);

        auto material { AssetDatabase::CreateMaterial(shader) };

        if (json.contains("textures")) {
            for (const auto& [uniformName, texEntry] : json.at("textures").items()) {
                const std::string texPath { texEntry.get<std::string>() };
                if (!texPath.empty()) {
                    material->SetTexture(uniformName, db.GetTexture(texPath));
                }
            }
        }

        if (json.contains("floats")) {
            for (const auto& [uniformName, value] : json.at("floats").items()) {
                material->SetFloat(uniformName, value.get<float>());
            }
        }

        if (json.contains("ints")) {
            for (const auto& [uniformName, value] : json.at("ints").items()) {
                material->SetInt(uniformName, value.get<int>());
            }
        }

        if (json.contains("vec3s")) {
            for (const auto& [uniformName, value] : json.at("vec3s").items()) {
                const auto arr = value.get<std::array<float, 3>>();
                material->SetVec3(uniformName, Vec3(arr[0], arr[1], arr[2]));
            }
        }

        if (json.contains("vec4s")) {
            for (const auto& [uniformName, value] : json.at("vec4s").items()) {
                const auto arr = value.get<std::array<float, 4>>();
                material->SetVec4(uniformName, Vec4(arr[0], arr[1], arr[2], arr[3]));
            }
        }

        return material;
    }

    static Json Serialize(const Bindables::Material& material) {
        Json json{};
        json["version"] = 1;
        json["shader"] = material.GetShader().GetPath();

        for (const auto& [name, slot] : material.GetTextures()) {
			const bool isEmbeddedTexture { slot.texture->GetPath().starts_with("*") };
        	if (slot.texture && !isEmbeddedTexture) {
        		json["textures"][name] = slot.texture->GetPath();
        	}
        }

        for (const auto& [name, value] : material.GetFloats()) {
            json["floats"][name] = value;
        }

        for (const auto& [name, value] : material.GetInts()) {
            json["ints"][name] = value;
        }

        for (const auto& [name, value] : material.GetVec3s()) {
            json["vec3s"][name] = { value.x, value.y, value.z };
        }

        for (const auto& [name, value] : material.GetVec4s()) {
            json["vec4s"][name] = { value.x, value.y, value.z, value.w };
        }

        return json;
    }
};

} // namespace Leadwort::AssetManagement