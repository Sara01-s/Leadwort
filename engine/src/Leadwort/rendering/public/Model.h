#pragma once

#include <Leadwort/core/public/Entity.h>
#include <Leadwort/rendering/bindables/public/Material.h>
#include <Leadwort/core/math/public/Color.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <memory>
#include <string>
#include <vector>

namespace Leadwort::Rendering::Bindables {
	class Mesh;
}

namespace Leadwort::Core {

	enum class PBRWorkflow {
		MetallicRoughness,
		SpecularGlossiness
	};

	struct MaterialFeatures {
	    bool hasDiffuse   { false };
	    bool hasNormals   { false };
	    bool hasSpecular  { false };
	    bool hasOpacity   { false };
	    bool hasEmissive  { false };
	    bool hasRoughness { false };
	    bool hasMetallic  { false };
	    bool hasAO        { false };
		PBRWorkflow pbrWorkflow { PBRWorkflow::MetallicRoughness };

	    float specularIntensity  { 1.0f };
	    float specularPower      { 1.0f };
		float roughnessIntensity { 0.5f };
		float metallicIntensity  { 0.0f };

	    Color color = Color::White();
	};

	class Model {
	public:
	    explicit Model(const std::string& path, AssetManagement::AssetKey<Model>);

	    Model(const Model&) = delete;
	    Model& operator=(const Model&) = delete;

	    void Instantiate(Entity& parentEntity);
		[[nodiscard ]] std::size_t GetMeshCount() const noexcept { return m_Meshes.size(); }
		[[nodiscard]] const Shared<Rendering::Bindables::Mesh>& GetMesh(const std::size_t index) const {
			LW_ASSERT(index < m_Meshes.size(), "Model::GetMesh: index out of range");
			return m_Meshes[index];
		}

	private:
	    void AttachNodeToEntity(const aiNode* node, Entity& entity);

	    Shared<Rendering::Bindables::Mesh> ParseMesh(const aiMesh* mesh, const aiScene* scene,
													 unsigned int meshIndex) const;

		static MaterialFeatures ParseMaterialFeatures(const aiMaterial* material);
	    void BindTextures(Rendering::Bindables::Material& material, const aiMaterial* aiMat, const MaterialFeatures& features) const;
		static Mat4 AssimpToMat4(const aiMatrix4x4& matrix);

	private:
	    Assimp::Importer m_Importer{};
	    const aiScene* m_AiScene { nullptr };
	    std::string m_ResourceBaseDir{};
	    std::vector<Shared<Rendering::Bindables::Mesh>> m_Meshes{};
		std::string m_FullPath{};
	};

} // namespace Engine::Game