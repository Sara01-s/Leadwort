#include "../public/PrimitiveMeshes.h"

#include "engine/asset-management/public/AssetManager.h"
#include "engine/asset-management/public/DefaultAssets.h"

#include <cmath>
#include <numbers>
#include <span>
#include <vector>

namespace Engine::Utils {

using namespace Engine::AssetManagement;
using namespace Engine::Rendering::Bindables;

PrimitiveMeshes::PrimitiveMeshes() {
    m_Layout
        .Append(ElementType::Position3D)
        .Append(ElementType::Normal3D)
        .Append(ElementType::TexCoord2D);
}

// ─────────────────────────────────────────────
//  Lazy accessors
// ─────────────────────────────────────────────

Shared<Mesh> PrimitiveMeshes::Quad()   { if (!m_Quad)   m_Quad   = BuildQuad();   return m_Quad; }
Shared<Mesh> PrimitiveMeshes::Plane()  { if (!m_Plane)  m_Plane  = BuildPlane();  return m_Plane; }
Shared<Mesh> PrimitiveMeshes::Sphere() { if (!m_Sphere) m_Sphere = BuildSphere(); return m_Sphere; }
Shared<Mesh> PrimitiveMeshes::Cube()   { if (!m_Cube)   m_Cube   = BuildCube();   return m_Cube; }

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────

static Shared<Mesh> RegisterMesh(
    const VertexLayout& layout,
    const std::span<const float> vertices,
    const std::span<const Index> indices,
    const char* key
) {
    return EngineAssets::GetMesh(MeshData { 
        .layout   = layout,
        .vertices = std::as_bytes(std::span(vertices)),
        .indices  = std::as_bytes(std::span(indices)),
        .material = DefaultAssets::CreateLitMaterial(),
        .key      = MeshKey::CreateAtRuntime(key)
    });
}

// ─────────────────────────────────────────────
//  Builders
// ─────────────────────────────────────────────

Shared<Mesh> PrimitiveMeshes::BuildQuad() const {
	constexpr std::array<float, 4 * 8> vertices = {
		-1,-1, 0,  0,0,1,  0,0,
		 1,-1, 0,  0,0,1,  1,0,
		 1, 1, 0,  0,0,1,  1,1,
		-1, 1, 0,  0,0,1,  0,1,
	};

	constexpr std::array<Index, 6> indices = { 0, 2, 1, 2, 0, 3 };

	return RegisterMesh(m_Layout, vertices, indices, "quad");
}

Shared<Mesh> PrimitiveMeshes::BuildPlane() const {
	constexpr float size = 5.0f;
	constexpr std::array<float, 4 * 8> vertices = {
		-size,0,-size,  0,1,0,  0,0,
		 size,0,-size,  0,1,0,  1,0,
		 size,0, size,  0,1,0,  1,1,
		-size,0, size,  0,1,0,  0,1,
	};

	constexpr std::array<Index, 6> indices = { 0, 1, 2, 2, 3, 0 };

	return RegisterMesh(m_Layout, vertices, indices, "plane");
}

Shared<Mesh> PrimitiveMeshes::BuildSphere() const {
    constexpr int   rings   = 20;
    constexpr int   sectors = 20;
    constexpr float pi      = std::numbers::pi_v<float>;

    std::vector<float> vertices;
    std::vector<Index> indices;

    vertices.reserve((rings + 1) * (sectors + 1) * 8);

    for (int i = 0; i <= rings; i++) {
        const float v   = static_cast<float>(i) / rings;
        const float phi = v * pi;

        for (int j = 0; j <= sectors; j++) {
            constexpr float radius = 0.5f;

            const float u     = static_cast<float>(j) / sectors;
            const float theta = u * pi * 2.0f;

            const float x = std::cos(theta) * std::sin(phi);
            const float y = std::cos(phi);
            const float z = std::sin(theta) * std::sin(phi);

            // pos
            vertices.push_back(x * radius);
            vertices.push_back(y * radius);
            vertices.push_back(z * radius);
            // normal
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            // uv
            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

	for (int i = 0; i < rings; i++) {
		for (int j = 0; j < sectors; j++) {
			const Index first  = i * (sectors + 1) + j;
			const Index second = first + sectors + 1;

			indices.insert(indices.end(), {
				first, first + 1, second,      // Triangle 1.
				second, first + 1, second + 1  // Triangle 2.
			});
		}
	}
    return RegisterMesh(m_Layout, vertices, indices, "sphere");
}

Shared<Mesh> PrimitiveMeshes::BuildCube() const {
    constexpr std::array<float, 24 * 8> vertices = {
        // pos                  normal        uv
        // Front (+Z)
        -0.5f,-0.5f, 0.5f,  0, 0, 1,  0,0,
         0.5f,-0.5f, 0.5f,  0, 0, 1,  1,0,
         0.5f, 0.5f, 0.5f,  0, 0, 1,  1,1,
        -0.5f, 0.5f, 0.5f,  0, 0, 1,  0,1,
        // Back (-Z)
         0.5f,-0.5f,-0.5f,  0, 0,-1,  0,0,
        -0.5f,-0.5f,-0.5f,  0, 0,-1,  1,0,
        -0.5f, 0.5f,-0.5f,  0, 0,-1,  1,1,
         0.5f, 0.5f,-0.5f,  0, 0,-1,  0,1,
        // Left (-X)
        -0.5f,-0.5f,-0.5f, -1, 0, 0,  0,0,
        -0.5f,-0.5f, 0.5f, -1, 0, 0,  1,0,
        -0.5f, 0.5f, 0.5f, -1, 0, 0,  1,1,
        -0.5f, 0.5f,-0.5f, -1, 0, 0,  0,1,
        // Right (+X)
         0.5f,-0.5f, 0.5f,  1, 0, 0,  0,0,
         0.5f,-0.5f,-0.5f,  1, 0, 0,  1,0,
         0.5f, 0.5f,-0.5f,  1, 0, 0,  1,1,
         0.5f, 0.5f, 0.5f,  1, 0, 0,  0,1,
        // Top (+Y)
        -0.5f, 0.5f, 0.5f,  0, 1, 0,  0,0,
         0.5f, 0.5f, 0.5f,  0, 1, 0,  1,0,
         0.5f, 0.5f,-0.5f,  0, 1, 0,  1,1,
        -0.5f, 0.5f,-0.5f,  0, 1, 0,  0,1,
        // Bottom (-Y)
        -0.5f,-0.5f,-0.5f,  0,-1, 0,  0,0,
         0.5f,-0.5f,-0.5f,  0,-1, 0,  1,0,
         0.5f,-0.5f, 0.5f,  0,-1, 0,  1,1,
        -0.5f,-0.5f, 0.5f,  0,-1, 0,  0,1,
    };

	constexpr std::array<Index, 36> indices = {
		0, 2, 1,  2, 0, 3,
		4, 6, 5,  6, 4, 7,
		8, 10, 9, 10, 8, 11,
	   12, 14, 13, 14, 12, 15,
	   16, 18, 17, 18, 16, 19,
	   20, 22, 21, 22, 20, 23,
   };

    return RegisterMesh(m_Layout, vertices, indices, "cube");
}

} // namespace Engine::Utils