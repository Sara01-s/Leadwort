#pragma once

#include "engine/rendering/bindables/public/Mesh.h"
#include "engine/rendering/bindables/public/Material.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Engine::Core  { class Entity; }

namespace Engine::Game {

struct MaterialFeatures {
    bool hasDiffuse   = false;
    bool hasNormals   = false;
    bool hasSpecular  = false;
    bool hasOpacity   = false;
    bool hasEmissive  = false;
    bool hasRoughness = false;
    bool hasMetallic  = false;
    bool hasAO        = false;

    Utils::Color color = Utils::Color(1, 1, 1, 1);
    float specularIntensity = 0.6f;
    float specularPower     = 32.0f;
};

class Model {
public:
    explicit Model(const std::string& path);

    Model(const Model&)            = delete;
    Model& operator=(const Model&) = delete;

    void Instantiate(Core::Entity& parentEntity);

private:
    void AttachNodeToEntity(const aiNode* node, Core::Entity& entity);

    std::shared_ptr<Rendering::Bindables::Mesh> ParseMesh(const aiMesh* mesh, const aiScene* scene) const;

    static MaterialFeatures ParseMaterialFeatures(const aiMaterial* material);
    void BindTextures(Rendering::Bindables::Material& material, const aiMaterial* aiMat, const MaterialFeatures& features) const;
    static glm::mat4 AssimpToGlm(const aiMatrix4x4& m);

    Assimp::Importer m_Importer;
    const aiScene* m_AiScene = nullptr;
    std::string m_ResourceBaseDir;
    std::vector<std::shared_ptr<Rendering::Bindables::Mesh>>  m_Meshes;
};

} // namespace Engine::Game