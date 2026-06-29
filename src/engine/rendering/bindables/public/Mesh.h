#pragma once

#include "Bindable.h"
#include "Material.h"
#include "MeshKey.h"
#include "VertexLayout.h"
#include "engine/core/math/public/AABB.h"
#include "engine/utils/public/Logger.h"

namespace Engine::Rendering::Bindables {

struct MeshData {
	const VertexLayout& layout;
	const ConstBufferView vertices;
	const ConstBufferView indices;
	Shared<Material> material;
	const MeshKey& key;
	GLenum topology = GL_TRIANGLES;
	GLenum usage = GL_DYNAMIC_DRAW;
};

class Mesh : public Bindable {
public:
	explicit Mesh(const MeshData& meshData, AssetManagement::AssetKey<Mesh>&& key) noexcept;

	~Mesh() override;
	void CalculateAABB(ConstBufferView vertexData);

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	static MeshKey GenerateKey(const std::string& path, const std::uint32_t index) noexcept {
		return { path, index };
	}

	void SetData(ConstBufferView vertexData, ConstBufferView indices) noexcept;

	void Render() const noexcept;
	void Bind() const noexcept override;
	void Unbind() const noexcept override;

	[[nodiscard]] constexpr int GetIndexCount() const noexcept { return m_IndexCount; }
	[[nodiscard]] Material* GetMaterial() const noexcept { return m_MeshData.material.get(); }
	[[nodiscard]] constexpr unsigned int GetTopology() const noexcept { return m_MeshData.topology; }
	[[nodiscard]] constexpr AABB GetAABB() const noexcept { return m_AABB; }
	[[nodiscard]] Shared<Material> GetSharedMaterial() const noexcept {
		CORE_ASSERT(reinterpret_cast<uintptr_t>(m_MeshData.material.get()) > 0x1000,
					"Mesh: A material pointer is garbage/uninitialized!");

		return m_MeshData.material;
	}

	void SetMaterial(const Shared<Material>& material) { m_MeshData.material = material; }

private:
	MeshData m_MeshData;
	AABB m_AABB{};

	uint32_t m_VAO = GL_INVALID_INDEX;
	uint32_t m_VBO = GL_INVALID_INDEX;
	uint32_t m_IBO = GL_INVALID_INDEX;
	int m_IndexCount = 0;
};

} // namespace Engine::Rendering::Bindables
