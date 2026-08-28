#pragma once

#include "IBindable.h"
#include "Material.h"
#include "MeshKey.h"
#include "VertexLayout.h"
#include <Leadwort/core/math/public/AABB.h>
#include <Leadwort/utils/public/Logger.h>

namespace Leadwort::Rendering::Bindables {

// Layout and key are held by value on purpose: a Mesh keeps its MeshData for its whole
// life, while callers build these from locals that die as soon as the mesh is created.
// The buffer views are the exception, they are only read during the upload and cleared
// right after, so nothing outlives the staging buffers it points at.
struct MeshData {
	std::string name{};
	VertexLayout layout{};
	ConstBufferView vertices{};
	ConstBufferView indices{};
	Shared<Material> material{};
	MeshKey key{};
	GLenum topology { GL_TRIANGLES };
	GLenum usage { GL_DYNAMIC_DRAW };
};

class Mesh : public IBindable {
public:
	explicit Mesh(const MeshData& meshData, AssetManagement::AssetKey<Mesh>&& key) noexcept;
	~Mesh() override;

	static MeshKey GenerateKey(const std::string& path, const std::uint32_t index) noexcept {
		return { path, index };
	}

	void CalculateAABB(ConstBufferView vertexData);
	void SetData(ConstBufferView vertexData, ConstBufferView indices) noexcept;

	void Render() const noexcept;
	void Bind() const noexcept override;
	void Unbind() const noexcept override;

	[[nodiscard]] constexpr std::string GetName() const noexcept { return m_MeshData.name; }
	[[nodiscard]] constexpr int GetIndexCount() const noexcept { return m_IndexCount; }
	[[nodiscard]] constexpr int GetVertexCount() const noexcept { return m_VertexCount; }
	[[nodiscard]] Material* GetMaterial() const noexcept { return m_MeshData.material.get(); }
	[[nodiscard]] constexpr unsigned int GetTopology() const noexcept { return m_MeshData.topology; }
	[[nodiscard]] constexpr AABB GetAABB() const noexcept { return m_AABB; }
	[[nodiscard]] Shared<Material> GetSharedMaterial() const noexcept {
		LW_ASSERT(reinterpret_cast<uintptr_t>(m_MeshData.material.get()) > 0x1000,
					"Mesh: A material pointer is garbage/uninitialized!");

		return m_MeshData.material;
	}

	void SetMaterial(const Shared<Material>& material) { m_MeshData.material = material; }
	void SetTopology(const GLenum topology) { m_MeshData.topology = topology; }

private:
	MeshData m_MeshData;
	AABB m_AABB{};

	uint32_t m_VAO { GL_INVALID_INDEX };
	uint32_t m_VBO { GL_INVALID_INDEX };
	uint32_t m_IBO { GL_INVALID_INDEX };
	int m_IndexCount { 0 };
	int m_VertexCount { 0 };
};

} // namespace Engine::Rendering::Bindables
