#pragma once

#include "Bindable.h"
#include "VertexLayout.h"
#include "Material.h"
#include <glad/glad.h>
#include <vector>
#include <cstdint>
#include <memory>

namespace Engine::Rendering::Bindables {

class Mesh : public Bindable {
public:
    Mesh(
        const VertexLayout&           layout,
        const void*                   vertexData,
        size_t                        vertexDataSize,
        const uint32_t*               indices,
        size_t                        indexCount,
        const std::shared_ptr<Material>& material,
        GLenum topology = GL_TRIANGLES,
        GLenum usage    = GL_DYNAMIC_DRAW
    );

    template <typename TVertex>
    Mesh(
        const VertexLayout&                  layout,
        const std::vector<TVertex>&          vertices,
        const std::vector<uint32_t>&         indices,
        std::shared_ptr<Material>            material,
        const GLenum topology = GL_TRIANGLES,
        const GLenum usage    = GL_DYNAMIC_DRAW
    ) : Mesh(
        layout,
        vertices.data(),
        vertices.size() * sizeof(TVertex),
        indices.data(),
        indices.size(),
        std::move(material),
        topology,
        usage
    ) {}

    ~Mesh() override;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void SetData(const void* vertexData, size_t vertexDataSize, const uint32_t* indices, size_t indexCount);

    template <typename TVertex>
    void SetData(const std::vector<TVertex>& vertices, const std::vector<uint32_t>& indices) {
        SetData(vertices.data(), vertices.size() * sizeof(TVertex), indices.data(), indices.size());
    }

    void Draw()   const;
    void Bind()   const noexcept override;
    void Unbind() const noexcept override;

    [[nodiscard]] int          GetIndexCount() const noexcept { return m_IndexCount; }
    [[nodiscard]] Material*    GetMaterial()   const noexcept { return m_Material.get(); }
    [[nodiscard]] unsigned int GetTopology()   const noexcept { return m_Topology; }

    void SetMaterial(const std::shared_ptr<Material>& material) { m_Material = material; }

private:
    VertexLayout              m_Layout;
    std::shared_ptr<Material> m_Material;
    GLenum                    m_Topology;
    GLenum                    m_Usage;

    uint32_t m_VAO        = GL_INVALID_INDEX;
    uint32_t m_VBO        = GL_INVALID_INDEX;
    uint32_t m_IBO        = GL_INVALID_INDEX;
    int      m_IndexCount = 0;
};

} // namespace Engine::Rendering::Bindables