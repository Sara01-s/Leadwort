#include "engine/rendering/public/Model.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "engine/asset-management/public/AssetManager.h"
#include "engine/asset-management/public/DefaultAssets.h"
#include "engine/components/public/MeshRenderer.h"
#include "engine/core/public/Entity.h"
#include "engine/rendering/bindables/public/Material.h"
#include "engine/rendering/bindables/public/Mesh.h"
#include "engine/rendering/bindables/public/VertexLayout.h"
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <vector>


namespace Engine::Core {

using namespace Rendering::Bindables;
using namespace AssetManagement;

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

Model::Model(const std::string& path, AssetKey<Model>) {
    std::string sanitizedPath = path;
    std::ranges::replace(sanitizedPath, '\\', '/');
    m_ResourceBaseDir = sanitizedPath.substr(0, sanitizedPath.find_last_of('/'));

    m_AiScene = m_Importer.ReadFile(path,
        aiProcess_Triangulate
        | aiProcess_JoinIdenticalVertices
        | aiProcess_CalcTangentSpace
        | aiProcess_ImproveCacheLocality
        | aiProcess_LimitBoneWeights
        | aiProcess_FindInvalidData
    );

    if (!m_AiScene) {
        throw std::runtime_error("Assimp Error: " + std::string(m_Importer.GetErrorString()));
    }

    for (unsigned int i = 0; i < m_AiScene->mNumMeshes; i++) {
        m_Meshes.push_back(ParseMesh(m_AiScene->mMeshes[i], m_AiScene, m_ResourceBaseDir));
    }
}

// ---------------------------------------------------------------------------
// Instantiation
// ---------------------------------------------------------------------------

void Model::Instantiate(Entity& parentEntity) {
    AttachNodeToEntity(m_AiScene->mRootNode, parentEntity);
}

void Model::AttachNodeToEntity(const aiNode* node, Entity& entity) {
	const unsigned int numMeshes = node->mNumMeshes;

	for (unsigned int i = 0; i < numMeshes; i++) {
		auto& mesh = m_Meshes[node->mMeshes[i]];

		if (numMeshes == 1) {
			auto* renderer = entity.AddComponent<Components::MeshRenderer>();
			renderer->mesh = mesh;
		}
		else {
			const std::string childName = std::string(node->mName.C_Str()) + "_mesh_" + std::to_string(i);
			entity.CreateChild(childName, [&](Entity& child) {
				auto* renderer = child.AddComponent<Components::MeshRenderer>();
				renderer->mesh = mesh;
			});
		}
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		const aiNode* childNode = node->mChildren[i];

		entity.CreateChild(childNode->mName.C_Str(), [&](Entity& child) {
			const Mat4 childMatrix = AssimpToMat4(childNode->mTransformation);

			child.GetTransform().SetLocalPosition(childMatrix.GetTranslation());
			child.GetTransform().SetLocalRotation(childMatrix.GetRotation());
			child.GetTransform().SetLocalScale(childMatrix.GetScale());

			AttachNodeToEntity(childNode, child);
		});
	}
}

// ---------------------------------------------------------------------------
// Mesh parsing
// ---------------------------------------------------------------------------

Shared<Mesh> Model::ParseMesh(const aiMesh* mesh, const aiScene* scene, const std::string& path) const {
    CORE_ASSERT(mesh != nullptr, "Model::ParseMesh: aiMesh is null.");
    CORE_ASSERT(scene != nullptr, "Model::ParseMesh: aiScene is null.");
	CORE_ASSERT(mesh->mMaterialIndex < scene->mNumMaterials, "Model::ParseMesh: Material index out of range.");
	CORE_ASSERT(scene->mMaterials[mesh->mMaterialIndex] != nullptr, "Model::ParseMesh: aiMaterial at index is null.");
    CORE_ASSERT(mesh->mVertices != nullptr, "Model::ParseMesh: Mesh does not contain vertices.");
    CORE_ASSERT(mesh->mMaterialIndex < scene->mNumMaterials, "Model::ParseMesh: Material index out of range.");

    using namespace Rendering::Bindables;

    const bool hasNormals   = mesh->HasNormals();
    const bool hasTexCoords = mesh->mTextureCoords[0] != nullptr;
    const bool hasTangents  = mesh->HasTangentsAndBitangents();

	const unsigned int meshIndex = mesh->mMaterialIndex;

    VertexLayout layout;
    layout.Append(ElementType::Position3D);
    if (hasNormals)   { layout.Append(ElementType::Normal3D);    }
    if (hasTexCoords) { layout.Append(ElementType::TexCoord2D);  }
    if (hasTangents)  { layout.Append(ElementType::Tangent3D);
                        layout.Append(ElementType::Bitangent3D); }

    std::vector<float> vertices{};
    vertices.reserve(mesh->mNumVertices * (layout.GetStride() / sizeof(float)));

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);

        if (hasNormals) {
            CORE_ASSERT(mesh->mNormals != nullptr, "Model::ParseMesh: Normals buffer is null.");
            vertices.push_back(mesh->mNormals[i].x);
            vertices.push_back(mesh->mNormals[i].y);
            vertices.push_back(mesh->mNormals[i].z);
        }

        if (hasTexCoords) {
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        }

        if (hasTangents) {
            CORE_ASSERT(mesh->mTangents != nullptr && mesh->mBitangents != nullptr, "Model::ParseMesh: Tangents/Bitangents buffer is null.");
            vertices.push_back(mesh->mTangents[i].x);
            vertices.push_back(mesh->mTangents[i].y);
            vertices.push_back(mesh->mTangents[i].z);
            vertices.push_back(mesh->mBitangents[i].x);
            vertices.push_back(mesh->mBitangents[i].y);
            vertices.push_back(mesh->mBitangents[i].z);
        }
    }

    std::vector<Index> indices{};
    indices.reserve(mesh->mNumFaces * 3);

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    const MaterialFeatures features = ParseMaterialFeatures(scene->mMaterials[meshIndex]);

	std::set<std::string> defines{};

	if (features.pbrWorkflow == PBRWorkflow::SpecularGlossiness) {
		defines.insert("SPECULAR_GLOSSINESS");
	}

	if (features.hasNormals && hasTangents) {
		defines.insert("HAS_NORMAL");
		defines.insert("HAS_TANGENTS");
	}

	if (features.hasDiffuse)   defines.insert("HAS_DIFFUSE");
	if (features.hasSpecular)  defines.insert("HAS_SPECULAR");
	if (features.hasOpacity)   defines.insert("HAS_OPACITY");
	if (features.hasEmissive)  defines.insert("HAS_EMISSIVE");
	if (features.hasRoughness) defines.insert("HAS_ROUGHNESS");
	if (features.hasMetallic)  defines.insert("HAS_METALLIC");
	if (features.hasAO)        defines.insert("HAS_AO");

	const Shared<Shader> shader = EngineAssets::GetShader("shaders/shd_lit.glsl", defines);
	const Shared<Material> material = EngineAssets::CreateMaterial(shader);

    material->SetColor4("_Color", features.color);
    material->SetFloat("_SpecularIntensity", features.specularIntensity);
    material->SetFloat("_SpecularPower", features.specularPower);
	material->SetFloat("_RoughnessIntensity", features.roughnessIntensity);
	material->SetFloat("_MetallicIntensity", features.metallicIntensity);

    BindTextures(*material, scene->mMaterials[meshIndex], features);

	const auto meshKey = MeshKey { path, meshIndex };
	auto resultMesh = EngineAssets::GetMesh(MeshData {
		layout,
		std::as_bytes(std::span<const float> { vertices }),
		std::as_bytes(std::span<const Index> { indices }),
		material,
		meshKey
	});

    CORE_ASSERT(resultMesh != nullptr, "Model::ParseMesh: Failed to instantiate mesh.");
    CORE_ASSERT(resultMesh->GetMaterial() != nullptr, "Model::ParseMesh: Created mesh has a null material.");

    return resultMesh;
}

// ---------------------------------------------------------------------------
// Material helpers
// ---------------------------------------------------------------------------
MaterialFeatures Model::ParseMaterialFeatures(const aiMaterial* material) {
    MaterialFeatures features{};

    const bool hasORM       = material->GetTextureCount(aiTextureType_UNKNOWN)   > 0;
    const bool hasSGSpecular = material->GetTextureCount(aiTextureType_SPECULAR) > 0;
    const bool hasMRMetallic = hasORM
                            || material->GetTextureCount(aiTextureType_METALNESS) > 0;

    features.pbrWorkflow = hasSGSpecular && !hasMRMetallic
        ? PBRWorkflow::SpecularGlossiness
        : PBRWorkflow::MetallicRoughness;

    features.hasDiffuse  = material->GetTextureCount(aiTextureType_DIFFUSE)    > 0
                        || material->GetTextureCount(aiTextureType_BASE_COLOR) > 0;
    features.hasNormals  = material->GetTextureCount(aiTextureType_NORMALS)    > 0
                        || material->GetTextureCount(aiTextureType_HEIGHT)     > 0;
    features.hasSpecular = hasSGSpecular;
    features.hasOpacity  = material->GetTextureCount(aiTextureType_OPACITY)      > 0
                        || material->GetTextureCount(aiTextureType_DISPLACEMENT) > 0;
    features.hasEmissive = material->GetTextureCount(aiTextureType_EMISSIVE)     > 0;

    if (features.pbrWorkflow == PBRWorkflow::MetallicRoughness) {
        features.hasRoughness = hasORM || material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0;
        features.hasMetallic  = hasORM || material->GetTextureCount(aiTextureType_METALNESS)         > 0;
        features.hasAO        = hasORM || material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0
                                       || material->GetTextureCount(aiTextureType_LIGHTMAP)          > 0;

        float roughness = 1.0f;
        float metallic  = 1.0f;
        material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
        material->Get(AI_MATKEY_METALLIC_FACTOR,  metallic);
        features.roughnessIntensity = roughness;
        features.metallicIntensity  = metallic;
    }
	else {
		// S/G: roughness comes from specular texture's alpha, metallic doesn't exists.
        features.hasAO             = material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0
                                  || material->GetTextureCount(aiTextureType_LIGHTMAP)          > 0;
        features.roughnessIntensity = 1.0f;
        features.metallicIntensity  = 0.0f;
    }

    aiColor4D color{};
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS) {
        features.color = Utils::Color(color.r, color.g, color.b, color.a);
    }

    return features;
}


void Model::BindTextures(
    Material& material,
    const aiMaterial* aiMat,
    const MaterialFeatures& features
) const {
    auto tryBind = [&](const std::string& uniform, const int slot, const std::initializer_list<aiTextureType> types) {
        for (const aiTextureType type : types) {
            aiString aiPath{};

            if (aiMat->GetTexture(type, 0, &aiPath) != AI_SUCCESS) {
				continue;
			}

			const std::string pathStr = aiPath.C_Str();

            if (pathStr[0] == '*') {
                const int index = std::stoi(pathStr.substr(1));
                const aiTexture* embedded = m_AiScene->mTextures[index];
                const auto* data = reinterpret_cast<const uint8_t*>(embedded->pcData);
                const size_t size = embedded->mWidth;

                const Shared<Texture> texture = EngineAssets::GetEmbeddedTexture(index, data, size);
                CORE_ASSERT(texture, "Failed to load embedded texture at index: " + std::to_string(index));
                material.SetTexture(uniform, texture, slot);
                return;
            }

            const std::string fullPath = m_ResourceBaseDir + "/" + pathStr;
            const Shared<Texture> texture = EngineAssets::GetTextureAbsolute(fullPath);

            CORE_ASSERT(texture, "Failed to load texture at path: " + fullPath);
            material.SetTexture(uniform, texture, slot);

            return;
        }
    };

    // ORM: Occlusion (R) Roughness (G) Metallic (B) — packed, used in M/R workflow
    const std::initializer_list ormTypes = {
        aiTextureType_UNKNOWN,
        aiTextureType_AMBIENT_OCCLUSION,
        aiTextureType_DIFFUSE_ROUGHNESS,
        aiTextureType_METALNESS,
    };

    if (features.hasDiffuse)  { tryBind("_DiffuseTexture",  0, {aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE}); }
    if (features.hasNormals)  { tryBind("_NormalTexture",   2, {aiTextureType_NORMALS, aiTextureType_HEIGHT}); }
    if (features.hasOpacity)  { tryBind("_OpacityTexture",  3, {aiTextureType_OPACITY, aiTextureType_DISPLACEMENT}); }
    if (features.hasEmissive) { tryBind("_EmissiveTexture", 4, {aiTextureType_EMISSIVE}); }

    if (features.pbrWorkflow == PBRWorkflow::SpecularGlossiness) {
        // S/G: specular RGB = F0, specular Alpha = glossiness (= 1 - roughness)
        if (features.hasSpecular) { tryBind("_SpecularTexture", 1, {aiTextureType_SPECULAR}); }
        if (features.hasAO)       { tryBind("_AmbientOcclusionTexture", 7, {aiTextureType_AMBIENT_OCCLUSION, aiTextureType_LIGHTMAP}); }
    }
	else {
        // M/R: ORM packed or separate textures
        if (features.hasSpecular)  { tryBind("_SpecularTexture",          1, {aiTextureType_SPECULAR}); }
        if (features.hasRoughness) { tryBind("_RoughnessTexture",         5, ormTypes); }
        if (features.hasMetallic)  { tryBind("_MetallicTexture",          6, ormTypes); }
        if (features.hasAO)        { tryBind("_AmbientOcclusionTexture",  7, ormTypes); }
    }
}

// ---------------------------------------------------------------------------
// Utilities
// ---------------------------------------------------------------------------

[[nodiscard]] Mat4 Model::AssimpToMat4(const aiMatrix4x4& matrix) {
	return Mat4(
		matrix.a1, matrix.a2, matrix.a3, matrix.a4,
		matrix.b1, matrix.b2, matrix.b3, matrix.b4,
		matrix.c1, matrix.c2, matrix.c3, matrix.c4,
		matrix.d1, matrix.d2, matrix.d3, matrix.d4
	);
}

} // namespace Engine::Game