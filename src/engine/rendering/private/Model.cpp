#include "engine/rendering/public/Model.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "engine/asset-management/public/AssetManager.h"
#include "engine/components/public/MeshRenderer.h"
#include "engine/core/public/Entity.h"
#include "engine/rendering/bindables/public/Material.h"
#include "engine/rendering/bindables/public/Mesh.h"
#include "engine/rendering/bindables/public/VertexLayout.h"
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <vector>

namespace Engine::Game {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

Model::Model(const std::string& path) {
    std::string sanitizedPath = path;
    std::ranges::replace(sanitizedPath, '\\', '/');
    m_ResourceBaseDir = sanitizedPath.substr(0, sanitizedPath.find_last_of('/'));

    m_AiScene = m_Importer.ReadFile(path,
        aiProcess_Triangulate
        | aiProcess_JoinIdenticalVertices
        | aiProcess_CalcTangentSpace
        | aiProcess_GenSmoothNormals
        | aiProcess_ImproveCacheLocality
        | aiProcess_LimitBoneWeights
        | aiProcess_FindInvalidData
        | aiProcess_FlipUVs
    );

    if (!m_AiScene) {
        throw std::runtime_error("Assimp Error: " + std::string(m_Importer.GetErrorString()));
    }

    for (unsigned int i = 0; i < m_AiScene->mNumMeshes; ++i) {
        m_Meshes.push_back(ParseMesh(m_AiScene->mMeshes[i], m_AiScene));
    }
}

// ---------------------------------------------------------------------------
// Instantiation
// ---------------------------------------------------------------------------

void Model::Instantiate(Core::Entity& parentEntity) {
    AttachNodeToEntity(m_AiScene->mRootNode, parentEntity);
}

void Model::AttachNodeToEntity(const aiNode* node, Core::Entity& entity) {
    const unsigned int numMeshes = node->mNumMeshes;

    for (unsigned int i = 0; i < numMeshes; ++i) {
        auto& mesh = m_Meshes[node->mMeshes[i]];
        auto mat = std::shared_ptr<Rendering::Bindables::Material>(mesh->GetMaterial(), [](auto*){}); // non-owning alias

        if (numMeshes == 1) {
            const auto& renderer = entity.AddComponent<Components::MeshRenderer>();
            renderer->mesh = mesh.get();
            renderer->mesh->SetMaterial(mat);
        }
        else {
            const std::string childName = std::string(node->mName.C_Str()) + "_mesh_" + std::to_string(i);
            entity.CreateChild(childName, [&](Core::Entity& child) {
                const auto& renderer = child.AddComponent<Components::MeshRenderer>();
                renderer->mesh = mesh.get();
                renderer->mesh->SetMaterial(mat);
            });
        }
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        const aiNode* childNode = node->mChildren[i];

        entity.CreateChild(childNode->mName.C_Str(), [&](Core::Entity& child) {
            glm::vec3 pos, scale, skew;
            glm::quat rot;
            glm::vec4 perspective;
            glm::decompose(AssimpToGlm(childNode->mTransformation), scale, rot, pos, skew, perspective);

            child.transform->SetLocalPosition(pos);
            child.transform->SetLocalRotation(rot);
            child.transform->SetLocalScale(scale);

            AttachNodeToEntity(childNode, child);
        });
    }
}

// ---------------------------------------------------------------------------
// Mesh parsing
// ---------------------------------------------------------------------------

std::shared_ptr<Rendering::Bindables::Mesh> Model::ParseMesh(const aiMesh* mesh, const aiScene* scene) const {
    using namespace Rendering::Bindables;

    const bool hasNormals   = mesh->HasNormals();
    const bool hasTexCoords = mesh->mTextureCoords[0] != nullptr;
    const bool hasTangents  = mesh->HasTangentsAndBitangents();

    VertexLayout layout;
    layout.Append(ElementType::Position3D);
    if (hasNormals)   { layout.Append(ElementType::Normal3D);    }
    if (hasTexCoords) { layout.Append(ElementType::TexCoord2D);  }
    if (hasTangents)  { layout.Append(ElementType::Tangent3D);
                        layout.Append(ElementType::Bitangent3D); }

    std::vector<float> vertices;
    vertices.reserve(mesh->mNumVertices * (layout.GetStride() / sizeof(float)));

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);

        if (hasNormals) {
            vertices.push_back(mesh->mNormals[i].x);
            vertices.push_back(mesh->mNormals[i].y);
            vertices.push_back(mesh->mNormals[i].z);
        }

        if (hasTexCoords) {
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        }

        if (hasTangents) {
            vertices.push_back(mesh->mTangents[i].x);
            vertices.push_back(mesh->mTangents[i].y);
            vertices.push_back(mesh->mTangents[i].z);
            vertices.push_back(mesh->mBitangents[i].x);
            vertices.push_back(mesh->mBitangents[i].y);
            vertices.push_back(mesh->mBitangents[i].z);
        }
    }

    std::vector<uint32_t> indices;
    indices.reserve(mesh->mNumFaces * 3);

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    const MaterialFeatures features = ParseMaterialFeatures(scene->mMaterials[mesh->mMaterialIndex]);

    const std::string shaderPath = features.hasDiffuse ? "shaders/shd_lit.glsl" : "shaders/shd_unlit.glsl";
    auto shader = AssetManagement::GameAssets::LoadShader(shaderPath);

    if (features.hasDiffuse)   { shader->EnableDefine("HAS_DIFFUSE");   }
    if (features.hasNormals)   { shader->EnableDefine("HAS_TANGENTS");  }
    if (features.hasSpecular)  { shader->EnableDefine("HAS_SPECULAR");  }
    if (features.hasOpacity)   { shader->EnableDefine("HAS_OPACITY");   }
    if (features.hasEmissive)  { shader->EnableDefine("HAS_EMISSIVE");  }
    if (features.hasRoughness) { shader->EnableDefine("HAS_ROUGHNESS"); }
    if (features.hasMetallic)  { shader->EnableDefine("HAS_METALLIC");  }
    if (features.hasAO)        { shader->EnableDefine("HAS_AO");        }

    auto material = std::make_shared<Material>(shader);
    material->SetColor4("_Color", features.color);
    material->SetFloat("_SpecularIntensity", features.specularIntensity);
    material->SetFloat("_SpecularPower",     features.specularPower);

    BindTextures(*material, scene->mMaterials[mesh->mMaterialIndex], features);

    return std::make_shared<Mesh>(layout, vertices, indices, std::move(material));
}

// ---------------------------------------------------------------------------
// Material helpers
// ---------------------------------------------------------------------------

MaterialFeatures Model::ParseMaterialFeatures(const aiMaterial* material) {
    MaterialFeatures features;

    features.hasDiffuse   = material->GetTextureCount(aiTextureType_DIFFUSE)           > 0
                         || material->GetTextureCount(aiTextureType_BASE_COLOR)        > 0;
    features.hasNormals   = material->GetTextureCount(aiTextureType_NORMALS)           > 0
                         || material->GetTextureCount(aiTextureType_HEIGHT)            > 0;
    features.hasSpecular  = material->GetTextureCount(aiTextureType_SPECULAR)          > 0;
    features.hasOpacity   = material->GetTextureCount(aiTextureType_OPACITY)           > 0
                         || material->GetTextureCount(aiTextureType_DISPLACEMENT)      > 0;
    features.hasEmissive  = material->GetTextureCount(aiTextureType_EMISSIVE)          > 0;
    features.hasRoughness = material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0;
    features.hasMetallic  = material->GetTextureCount(aiTextureType_METALNESS)         > 0
                         || material->GetTextureCount(aiTextureType_UNKNOWN)           > 0;
    features.hasAO        = material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0
                         || material->GetTextureCount(aiTextureType_LIGHTMAP)          > 0
                         || material->GetTextureCount(aiTextureType_UNKNOWN)           > 0
                         || material->GetTextureCount(aiTextureType_SHININESS)         > 0;

    aiColor4D color;

    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS) {
        features.color = Utils::Color(color.r, color.g, color.b, color.a);
    }

    return features;
}

void Model::BindTextures(
    Rendering::Bindables::Material& material,
    const aiMaterial* aiMat,
    const MaterialFeatures& features
) const {
    auto tryBind = [&](const std::string& uniform, const int slot, const std::initializer_list<aiTextureType> types) {
        for (const aiTextureType type : types) {
            aiString path;

            if (aiMat->GetTexture(type, 0, &path) == AI_SUCCESS) {
                const std::string fullPath = m_ResourceBaseDir + "/" + path.C_Str();
                const auto texture = AssetManagement::EngineAssets::LoadTexture(fullPath);

                if (texture) {
                    material.SetTexture(uniform, texture.get(), slot);
                    return;
                }
            }
        }
    };

    if (features.hasDiffuse)   { tryBind("_DiffuseTexture",           0, {aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE}); }
    if (features.hasSpecular)  { tryBind("_SpecularTexture",          1, {aiTextureType_SPECULAR, aiTextureType_SHININESS}); }
    if (features.hasNormals)   { tryBind("_NormalTexture",            2, {aiTextureType_NORMALS, aiTextureType_HEIGHT}); }
    if (features.hasOpacity)   { tryBind("_OpacityTexture",           3, {aiTextureType_OPACITY, aiTextureType_DISPLACEMENT}); }
    if (features.hasEmissive)  { tryBind("_EmissiveTexture",          4, {aiTextureType_EMISSIVE}); }
    if (features.hasRoughness) { tryBind("_RoughnessTexture",         5, {aiTextureType_DIFFUSE_ROUGHNESS}); }
    if (features.hasMetallic)  { tryBind("_MetallicTexture",          6, {aiTextureType_METALNESS, aiTextureType_UNKNOWN}); }
    if (features.hasAO)        { tryBind("_AmbientOcclusionTexture",  7, {aiTextureType_AMBIENT_OCCLUSION, aiTextureType_LIGHTMAP,
                                                                           aiTextureType_UNKNOWN, aiTextureType_SHININESS}); }
}

// ---------------------------------------------------------------------------
// Utilities
// ---------------------------------------------------------------------------

glm::mat4 Model::AssimpToGlm(const aiMatrix4x4& m) {
    return glm::transpose(glm::make_mat4(&m.a1));
}

} // namespace Engine::Game