#include "../public/Mesh.h"

#include "engine/utils/public/Logger.h"

namespace Engine::Rendering::Bindables {

Mesh::Mesh(const MeshData& meshData, AssetManagement::AssetKey<Mesh>&&) : m_MeshData(meshData)
{
	CORE_ASSERT(m_MeshData.material != nullptr, "Mesh constructor: Material is NULL after initialization!");

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_IBO);

	// Set up attribute pointers once, VAO will remember them.
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);

	size_t offset = 0;
	for (const auto& attribute: m_MeshData.layout.GetAttributes()) {
		attribute.Apply(static_cast<int>(m_MeshData.layout.GetStride()), reinterpret_cast<const void*>(offset));
		offset += attribute.sizeInBytes;
	}

	glBindVertexArray(0);

	SetData(m_MeshData.vertices, m_MeshData.indices);
}

Mesh::~Mesh() {
	CORE_LOG("Destroying Mesh with GPU ID: ", m_GpuID);

	if (m_VBO) {
		glDeleteBuffers(1, &m_VBO);
		m_VBO = 0;
	}

	if (m_IBO) {
		glDeleteBuffers(1, &m_IBO);
		m_IBO = 0;
	}

	if (m_VAO) {
		glDeleteVertexArrays(1, &m_VAO);
		m_VAO = 0;
	}
}

void Mesh::SetData(const ConstBufferView vertexData, const ConstBufferView indices) {
	if (indices.empty()) {
		m_IndexCount = 0;
		return;
	}

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size_bytes(), vertexData.data(), m_MeshData.usage);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size_bytes(), indices.data(), m_MeshData.usage);

	glBindVertexArray(0);

	m_IndexCount = static_cast<int>(indices.size_bytes() / sizeof(Index));
}

void Mesh::Render() const {
	if (m_IndexCount <= 0) {
		CORE_WARN("Trying to render a mesh with IndexCount = 0!");
		return;
	}

	glBindVertexArray(m_VAO);
	glDrawElements(m_MeshData.topology, m_IndexCount, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void Mesh::Bind() const noexcept {
	glBindVertexArray(m_VAO);
}

void Mesh::Unbind() const noexcept {
	glBindVertexArray(0);
}

} // namespace Engine::Rendering::Bindables