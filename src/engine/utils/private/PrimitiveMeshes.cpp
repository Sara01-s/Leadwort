#include "../public/PrimitiveMeshes.h"

#include "engine/asset-management/public/DefaultAssets.h"

#include <cmath>
#include <glm/glm.hpp>
#include <numbers>

namespace Engine::Utils {

using namespace Engine::Rendering::Bindables;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

PrimitiveMeshes::PrimitiveMeshes() {
    m_Layout
        .Append(ElementType::Position3D)
        .Append(ElementType::Normal3D)
        .Append(ElementType::TexCoord2D);

	m_Material = AssetManagement::DefaultAssets::GetLitMaterial();
}

// ─────────────────────────────────────────────
//  Lazy accessors
// ─────────────────────────────────────────────

Mesh* PrimitiveMeshes::Empty()  { if (!m_Empty)  m_Empty  = BuildEmpty();  return m_Empty.get(); }
Mesh* PrimitiveMeshes::Quad()   { if (!m_Quad)   m_Quad   = BuildQuad();   return m_Quad.get(); }
Mesh* PrimitiveMeshes::Plane()  { if (!m_Plane)  m_Plane  = BuildPlane();  return m_Plane.get(); }
Mesh* PrimitiveMeshes::Sphere() { if (!m_Sphere) m_Sphere = BuildSphere(); return m_Sphere.get(); }
Mesh* PrimitiveMeshes::Cube()   { if (!m_Cube)   m_Cube   = BuildCube();   return m_Cube.get(); }

// ─────────────────────────────────────────────
//  Builders
// ─────────────────────────────────────────────

std::unique_ptr<Mesh> PrimitiveMeshes::BuildEmpty() const {
    const std::vector<Vertex> vertices = {
        { {0, 0, 0}, {0, 1, 0}, {0, 0} }
    };
    const std::vector<uint32_t> indices = { 0 };
	return std::make_unique<Mesh>(m_Layout, vertices, indices, m_Material);
}

std::unique_ptr<Mesh> PrimitiveMeshes::BuildQuad() const {
    constexpr glm::vec3 normal = { 0, 0, 1 };
    const std::vector<Vertex> vertices = {
        { {-1, -1, 0}, normal, {0, 0} },
        { { 1, -1, 0}, normal, {1, 0} },
        { { 1,  1, 0}, normal, {1, 1} },
        { {-1,  1, 0}, normal, {0, 1} },
    };
    const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };
	return std::make_unique<Mesh>(m_Layout, vertices, indices, m_Material);
}

std::unique_ptr<Mesh> PrimitiveMeshes::BuildPlane() const {
    constexpr float size = 5.0f;
    constexpr glm::vec3 normal = { 0, 1, 0 };
    const std::vector<Vertex> vertices = {
        { {-size, 0, -size}, normal, {0, 0} },
        { { size, 0, -size}, normal, {1, 0} },
        { { size, 0,  size}, normal, {1, 1} },
        { {-size, 0,  size}, normal, {0, 1} },
    };
    const std::vector<uint32_t> indices = { 0, 2, 1, 2, 0, 3 };
	return std::make_unique<Mesh>(m_Layout, vertices, indices, m_Material);
}

std::unique_ptr<Mesh> PrimitiveMeshes::BuildSphere() const {
    constexpr int   rings   = 20;
    constexpr int   sectors = 20;
    constexpr float pi      = std::numbers::pi_v<float>;

    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    for (int i = 0; i <= rings; ++i) {
        const float v   = static_cast<float>(i) / rings;
        const float phi = v * pi;

        for (int j = 0; j <= sectors; ++j) {
            constexpr float radius = 0.5f;
            const float u     = static_cast<float>(j) / sectors;
            const float theta = u * pi * 2.0f;

            const float x = std::cos(theta) * std::sin(phi);
            const float y = std::cos(phi);
            const float z = std::sin(theta) * std::sin(phi);

            vertices.push_back({ {x * radius, y * radius, z * radius}, {x, y, z}, {u, v} });
        }
    }

    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < sectors; ++j) {
            const uint32_t first  = (i * (sectors + 1)) + j;
            const uint32_t second = first + sectors + 1;
            indices.insert(indices.end(), { first, second, first + 1, second, second + 1, first + 1 });
        }
    }

	return std::make_unique<Mesh>(m_Layout, vertices, indices, m_Material);
}

std::unique_ptr<Mesh> PrimitiveMeshes::BuildCube() const {
    const std::vector<Vertex> vertices = {
        // Front (+Z)
        { {-0.5f, -0.5f,  0.5f}, {0, 0, 1}, {0, 0} }, { { 0.5f, -0.5f,  0.5f}, {0, 0, 1}, {1, 0} },
        { { 0.5f,  0.5f,  0.5f}, {0, 0, 1}, {1, 1} }, { {-0.5f,  0.5f,  0.5f}, {0, 0, 1}, {0, 1} },
        // Back (-Z)
        { { 0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 0} }, { {-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1, 0} },
        { {-0.5f,  0.5f, -0.5f}, {0, 0, -1}, {1, 1} }, { { 0.5f,  0.5f, -0.5f}, {0, 0, -1}, {0, 1} },
        // Left (-X)
        { {-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 0} }, { {-0.5f, -0.5f,  0.5f}, {-1, 0, 0}, {1, 0} },
        { {-0.5f,  0.5f,  0.5f}, {-1, 0, 0}, {1, 1} }, { {-0.5f,  0.5f, -0.5f}, {-1, 0, 0}, {0, 1} },
        // Right (+X)
        { { 0.5f, -0.5f,  0.5f}, {1, 0, 0}, {0, 0} }, { { 0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1, 0} },
        { { 0.5f,  0.5f, -0.5f}, {1, 0, 0}, {1, 1} }, { { 0.5f,  0.5f,  0.5f}, {1, 0, 0}, {0, 1} },
        // Top (+Y)
        { {-0.5f,  0.5f,  0.5f}, {0, 1, 0}, {0, 0} }, { { 0.5f,  0.5f,  0.5f}, {0, 1, 0}, {1, 0} },
        { { 0.5f,  0.5f, -0.5f}, {0, 1, 0}, {1, 1} }, { {-0.5f,  0.5f, -0.5f}, {0, 1, 0}, {0, 1} },
        // Bottom (-Y)
        { {-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 0} }, { { 0.5f, -0.5f, -0.5f}, {0, -1, 0}, {1, 0} },
        { { 0.5f, -0.5f,  0.5f}, {0, -1, 0}, {1, 1} }, { {-0.5f, -0.5f,  0.5f}, {0, -1, 0}, {0, 1} },
    };

    const std::vector<uint32_t> indices = {
         0,  1,  2,  2,  3,  0,
         4,  5,  6,  6,  7,  4,
         8,  9, 10, 10, 11,  8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20,
    };

	return std::make_unique<Mesh>(m_Layout, vertices, indices, m_Material);
}

} // namespace Engine::Utils