#include "../public/Mesh.h"

#include "engine/utils/public/Logger.h"

namespace Engine::Rendering::Bindables {

Mesh::Mesh(
	const VertexLayout& layout,
	const void* vertexData,
	const size_t vertexDataSize,
	const uint32_t* indices,
	const size_t indexCount,
	const std::shared_ptr<Material>& material,
	const GLenum topology,
	const GLenum usage
)
	: m_Layout(layout), m_Material(material), m_Topology(topology), m_Usage(usage)
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_IBO);

	// Set up attribute pointers once, VAO will remember them.
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);

	size_t offset = 0;
	for (const auto& attr: layout.GetAttributes()) {
		attr.Apply(static_cast<int>(layout.GetStride()), reinterpret_cast<const void*>(offset));
		offset += attr.sizeInBytes;
	}

	glBindVertexArray(0);

	SetData(vertexData, vertexDataSize, indices, indexCount);
}

Mesh::~Mesh() {
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

void Mesh::SetData(
	const void* vertexData,
	const size_t vertexDataSize,
	const uint32_t* indices,
	const size_t indexCount
) {
	if (indexCount == 0) {
		m_IndexCount = 0;
		return;
	}

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertexDataSize), vertexData, m_Usage);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indexCount * sizeof(uint32_t)), indices, m_Usage);

	glBindVertexArray(0);

	m_IndexCount = static_cast<int>(indexCount);
}

void Mesh::Draw() const {
	if (m_IndexCount <= 0) {
		CORE_WARN("Trying to render a mesh with IndexCount = 0!");
		return;
	}

	glBindVertexArray(m_VAO);
	glDrawElements(m_Topology, m_IndexCount, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void Mesh::Bind() const noexcept {
	glBindVertexArray(m_VAO);
}
void Mesh::Unbind() const noexcept {
	glBindVertexArray(0);
}

} // namespace Engine::Rendering::Bindables