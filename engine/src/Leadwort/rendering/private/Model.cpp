#include <Leadwort/rendering/public/Model.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <Leadwort/asset-management/public/AssetDatabase.h>
#include <Leadwort/asset-management/public/DefaultAssets.h>
#include <Leadwort/components/public/MeshRenderer.h>
#include <Leadwort/components/public/Transform.h>
#include <Leadwort/core/public/Entity.h>
#include <Leadwort/rendering/bindables/public/Material.h>
#include <Leadwort/rendering/bindables/public/Mesh.h>
#include <Leadwort/rendering/bindables/public/VertexLayout.h>
#include <assimp/GltfMaterial.h>
#include <assimp/config.h>
#include <algorithm>
#include <string_view>
#include <vector>


namespace Leadwort::Core {

	using namespace Rendering::Bindables;
	using namespace AssetManagement;

	// ---------------------------------------------------------------------------
	// Construction
	// ---------------------------------------------------------------------------

	Model::Model(const std::string& path, AssetKey<Model>) {
		std::string sanitizedPath = path;
		std::ranges::replace(sanitizedPath, '\\', '/');
		m_ResourceBaseDir = sanitizedPath.substr(0, sanitizedPath.find_last_of('/'));
		m_FullPath = sanitizedPath;

	    // Points and lines have no place in a mesh renderer, and a 1- or 2-index "face"
	    // would silently desync the whole index buffer, so drop those primitives at
	    // import time instead of trusting the data.
	    m_Importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

	    m_AiScene = m_Importer.ReadFile(path,
	        aiProcess_Triangulate
	        | aiProcess_SortByPType
	        // Only fires on meshes that arrived without normals. Without it their vertex
	        // layout skips a slot and every later attribute lands on the wrong location.
	        | aiProcess_GenSmoothNormals
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
			m_Meshes.push_back(ParseMesh(m_AiScene->mMeshes[i], m_AiScene, i));
		}
	}

	// ---------------------------------------------------------------------------
	// Instantiation
	// ---------------------------------------------------------------------------

	// Push a node's local matrix onto a transform. The decomposition is only valid for a
	// TRS matrix, which is what glTF nodes are (shear cannot be expressed there).
	static void ApplyLocalMatrix(Components::Transform& transform, const Mat4& matrix) {
		transform.SetLocalMatrix(matrix);
	}

	void Model::Instantiate(Entity& parentEntity) {
		// Assimp only synthesises an identity root when the file has several root nodes.
		// A single-root export — one Blender object, or one parent holding the rest — puts
		// that object's own transform on mRootNode, and ignoring it would drop the model at
		// the target's origin instead of where Blender had it. Compose rather than
		// overwrite, so the transform the caller already gave the target survives.
		const Mat4 rootMatrix { AssimpToMat4(m_AiScene->mRootNode->mTransformation) };

		if (rootMatrix != Mat4::Identity()) {
			auto& transform { parentEntity.GetTransform() };
			ApplyLocalMatrix(transform, transform.GetLocalMatrix() * rootMatrix);
		}

	    AttachNodeToEntity(m_AiScene->mRootNode, parentEntity);
	}

	void Model::AttachNodeToEntity(const aiNode* node, Entity& entity) {
		const unsigned int numMeshes { node->mNumMeshes };

		for (std::uint32_t i = 0; i < numMeshes; ++i) {
			const uint32_t meshIndexInModel = node->mMeshes[i];
			auto& mesh { m_Meshes[meshIndexInModel] };

			auto setupRenderer = [&](Components::MeshRenderer* renderer) {
				renderer->mesh = mesh;
				renderer->isPrimitive = false;
				renderer->modelPath = m_FullPath;
				renderer->meshIndex = static_cast<int>(meshIndexInModel);
				renderer->SyncRenderQueueFromMaterial();
			};

			if (numMeshes == 1) {
				auto* renderer { entity.AddComponent<Components::MeshRenderer>() };
				setupRenderer(renderer);
			}
			else {
				// A glTF mesh with several primitives (one per material slot) arrives as
				// several aiMeshes on the same node, so each gets its own entity. They carry
				// no transform of their own: they sit exactly on the node that owns them.
				const std::string childName { std::string(node->mName.C_Str()) + "_mesh_" + std::to_string(i) };
				entity.CreateChild(childName, [&](Entity& child) {
				   auto* renderer { child.AddComponent<Components::MeshRenderer>() };
				   setupRenderer(renderer);
				});
			}
		}

		for (unsigned int i = 0; i < node->mNumChildren; ++i) {
			const aiNode* childNode { node->mChildren[i] };

			entity.CreateChild(childNode->mName.C_Str(), [&](Entity& child) {
			   ApplyLocalMatrix(child.GetTransform(), AssimpToMat4(childNode->mTransformation));

			   AttachNodeToEntity(childNode, child);
			});
		}
	}

	// ---------------------------------------------------------------------------
	// Mesh parsing
	// ---------------------------------------------------------------------------

	// glTF's alphaMode maps one to one onto the engine's queues.
	static Rendering::RenderQueue ToRenderQueue(const AlphaMode alphaMode) noexcept {
		switch (alphaMode) {
			case AlphaMode::Mask:   return Rendering::RenderQueue::AlphaTest;
			case AlphaMode::Blend:  return Rendering::RenderQueue::Transparent;
			case AlphaMode::Opaque: break;
		}

		return Rendering::RenderQueue::Opaque;
	}

	// MASK is opaque geometry with a discard: depth writes stay on and blending stays
	// off. Only BLEND drops depth writes and blends, which is also what makes
	// DrawCommand::Create sort it back-to-front.
	static Rendering::RenderPipelineState BuildPipelineState(const MaterialFeatures& features) noexcept {
		Rendering::RenderPipelineState state {
			features.alphaMode == AlphaMode::Blend
				? Rendering::RenderPipelineState::Transparent()
				: Rendering::RenderPipelineState::Opaque()
		};

		state.cullMode = features.doubleSided ? Rendering::CullMode::None : Rendering::CullMode::Back;

		return state;
	}

	Shared<Mesh> Model::ParseMesh(const aiMesh* mesh, const aiScene* scene, unsigned int meshIndex) const {
	    LW_ASSERT(mesh != nullptr, "Model::ParseMesh: aiMesh is null.");
	    LW_ASSERT(scene != nullptr, "Model::ParseMesh: aiScene is null.");
		LW_ASSERT(mesh->mMaterialIndex < scene->mNumMaterials, "Model::ParseMesh: Material index out of range.");
		LW_ASSERT(scene->mMaterials[mesh->mMaterialIndex] != nullptr, "Model::ParseMesh: aiMaterial at index is null.");
	    LW_ASSERT(mesh->mVertices != nullptr, "Model::ParseMesh: Mesh does not contain vertices.");
	    LW_ASSERT(mesh->mMaterialIndex < scene->mNumMaterials, "Model::ParseMesh: Material index out of range.");

	    using namespace Rendering::Bindables;

	    const bool hasNormals   { mesh->HasNormals() };
	    const bool hasTexCoords { mesh->mTextureCoords[0] != nullptr };
	    const bool hasTangents  { mesh->HasTangentsAndBitangents() };

		const unsigned int materialIndex { mesh->mMaterialIndex };

	    VertexLayout layout{};
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
	            LW_ASSERT(mesh->mNormals != nullptr, "Model::ParseMesh: Normals buffer is null.");
	            vertices.push_back(mesh->mNormals[i].x);
	            vertices.push_back(mesh->mNormals[i].y);
	            vertices.push_back(mesh->mNormals[i].z);
	        }

	        if (hasTexCoords) {
	            vertices.push_back(mesh->mTextureCoords[0][i].x);
	            vertices.push_back(mesh->mTextureCoords[0][i].y);
	        }

	        if (hasTangents) {
	            LW_ASSERT(mesh->mTangents != nullptr && mesh->mBitangents != nullptr, "Model::ParseMesh: Tangents/Bitangents buffer is null.");
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

	        // Triangulate + SBP_REMOVE should leave nothing else, but a stray non-triangle
	        // would shift every index after it, so skip instead of appending blindly.
	        if (face.mNumIndices != 3) {
	            continue;
	        }

	        for (unsigned int j = 0; j < face.mNumIndices; j++) {
	            indices.push_back(face.mIndices[j]);
	        }
	    }

	    const MaterialFeatures features { ParseMaterialFeatures(scene->mMaterials[materialIndex]) };

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

		const auto meshKey { MeshKey { m_FullPath, meshIndex } };

		const Shared<Shader> shader { EngineAssets::GetShader("shaders/shd_lit.glsl", defines) };
		const Shared<Material> material { EngineAssets::GetOrCreateMaterial(meshKey, shader) };

		material->SetName(scene->mMaterials[materialIndex]->GetName().C_Str());

	    material->SetColor4("_Color", features.color);
	    material->SetFloat("_SpecularIntensity", features.specularIntensity);
	    material->SetFloat("_SpecularPower", features.specularPower);
		material->SetFloat("_RoughnessIntensity", features.roughnessIntensity);
		material->SetFloat("_MetallicIntensity", features.metallicIntensity);

		material->renderQueue = ToRenderQueue(features.alphaMode);
		material->pipelineState = BuildPipelineState(features);

		// A cutoff of zero disables the test in the shader, which is what OPAQUE (alpha
		// ignored) and BLEND (alpha blended, never discarded) both want.
		material->SetFloat("_AlphaCutoff", features.alphaMode == AlphaMode::Mask ? features.alphaCutoff : 0.0f);

	    BindTextures(*material, scene->mMaterials[materialIndex], features);

		auto resultMesh { EngineAssets::GetMesh(MeshData {
			std::string(mesh->mName.C_Str()),
			layout,
			std::as_bytes(std::span<const float> { vertices }),
			std::as_bytes(std::span<const Index> { indices }),
			material,
			meshKey
		}) };

	    LW_ASSERT(resultMesh != nullptr, "Model::ParseMesh: Failed to instantiate mesh.");
	    LW_ASSERT(resultMesh->GetMaterial() != nullptr, "Model::ParseMesh: Created mesh has a null material.");

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
	        features.color = Color(color.r, color.g, color.b, color.a);
	    }

	    aiString alphaMode{};
	    if (material->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == AI_SUCCESS) {
	        const std::string_view mode { alphaMode.C_Str() };

	        if (mode == "MASK") {
	            features.alphaMode = AlphaMode::Mask;
	        }
	        else if (mode == "BLEND") {
	            features.alphaMode = AlphaMode::Blend;
	        }
	    }

	    float alphaCutoff { 0.5f };
	    if (material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff) == AI_SUCCESS) {
	        features.alphaCutoff = alphaCutoff;
	    }

	    // glTF doubleSided. Foliage cards are the reason this matters: culled backfaces
	    // make half of every leaf disappear.
	    int twoSided { 0 };
	    if (material->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS) {
	        features.doubleSided = twoSided != 0;
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

	                const Shared<Texture> texture = EngineAssets::GetEmbeddedTexture(m_FullPath, index, data, size);
	                LW_ASSERT(texture, "Failed to load embedded texture at index: " + std::to_string(index));
	                material.SetTexture(uniform, texture, slot);
	                return;
	            }

	            const std::string fullPath = m_ResourceBaseDir + "/" + pathStr;
	            const Shared<Texture> texture = EngineAssets::GetTextureFromAbsolutePath(fullPath);

	            LW_ASSERT(texture, "Failed to load texture at path: " + fullPath);
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