#pragma once

#include "engine/core/public/Entity.h"
#include "engine/rendering/bindables/public/Material.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <memory>
#include <string>
#include <vector>

namespace Engine::Rendering::Bindables {
	class Mesh;
}

namespace Engine::Core {

struct MaterialFeatures {
    bool hasDiffuse   = false;
    bool hasNormals   = false;
    bool hasSpecular  = false;
    bool hasOpacity   = false;
    bool hasEmissive  = false;
    bool hasRoughness = false;
    bool hasMetallic  = false;
    bool hasAO        = false;

    float specularIntensity  = 1.0f;
    float specularPower      = 1.0f;
	float roughnessIntensity = 0.5f;
	float metallicIntensity  = 0.0f;

    Utils::Color color = Utils::Color::White();
};

class Model {
public:
    explicit Model(const std::string& path, AssetManagement::AssetKey<Model>);

    Model(const Model&)            = delete;
    Model& operator=(const Model&) = delete;

    void Instantiate(Entity& parentEntity);

private:
    void AttachNodeToEntity(const aiNode* node, Entity& entity);

    Shared<Rendering::Bindables::Mesh> ParseMesh(const aiMesh* mesh, const aiScene* scene, const std::string& path) const;

    static MaterialFeatures ParseMaterialFeatures(const aiMaterial* material);
    void BindTextures(Rendering::Bindables::Material& material, const aiMaterial* aiMat, const MaterialFeatures& features) const;
	static Mat4 AssimpToMat4(const aiMatrix4x4& m);

    Assimp::Importer m_Importer;
    const aiScene* m_AiScene = nullptr;
    std::string m_ResourceBaseDir;
    std::vector<Shared<Rendering::Bindables::Mesh>> m_Meshes;
};

} // namespace Engine::Game