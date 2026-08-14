#include "../public/PrimitiveMeshes.h"

#include <Leadwort/asset-management/public/AssetDatabase.h>
#include <Leadwort/asset-management/public/DefaultAssets.h>

#include <cmath>
#include <numbers>
#include <span>
#include <vector>

namespace Leadwort::Utils {

	using namespace Leadwort::AssetManagement;
	using namespace Leadwort::Rendering::Bindables;

	PrimitiveMeshes::PrimitiveMeshes() {
	    m_Layout
	        .Append(ElementType::Position3D)
	        .Append(ElementType::Normal3D)
	        .Append(ElementType::TexCoord2D);

		Empty();
		Quad();
		Plane();
		Sphere();
		Cube();
		Capsule();
	}

	// ─────────────────────────────────────────────
	//  Lazy accessors
	// ─────────────────────────────────────────────

	Shared<Mesh> PrimitiveMeshes::Empty()  { if (!m_Empty)  m_Empty  = BuildEmpty();  return m_Empty;}
	Shared<Mesh> PrimitiveMeshes::Quad()   { if (!m_Quad)   m_Quad   = BuildQuad();   return m_Quad; }
	Shared<Mesh> PrimitiveMeshes::Plane()  { if (!m_Plane)  m_Plane  = BuildPlane();  return m_Plane; }
	Shared<Mesh> PrimitiveMeshes::Sphere() { if (!m_Sphere) m_Sphere = BuildSphere(); return m_Sphere; }
	Shared<Mesh> PrimitiveMeshes::Cube()   { if (!m_Cube)   m_Cube   = BuildCube();   return m_Cube; }
	Shared<Mesh> PrimitiveMeshes::Capsule() { if (!m_Capsule) m_Capsule = BuildCapsule(); return m_Capsule; }

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
	    	.name	  = key,
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
	Shared<Mesh> PrimitiveMeshes::BuildEmpty() const noexcept {
		constexpr std::array<float, 4 * 8> vertices {
			0,0,0,  0,1,0,  0,0,
			0,0,0,  0,1,0,  1,0,
			0,0,0,  0,1,0,  1,1,
			0,0,0,  0,1,0,  0,1,
		};

		constexpr std::array<Index, 6> indices { 0, 2, 1, 2, 0, 3 };

		return RegisterMesh(m_Layout, vertices, indices, "Empty");
	}

	Shared<Mesh> PrimitiveMeshes::BuildQuad() const noexcept {
		constexpr std::array<float, 4 * 8> vertices {
			-1,-1, 0,  0,0,1,  0,0,
			 1,-1, 0,  0,0,1,  1,0,
			 1, 1, 0,  0,0,1,  1,1,
			-1, 1, 0,  0,0,1,  0,1,
		};

		constexpr std::array<Index, 6> indices { 0, 2, 1, 2, 0, 3 };

		return RegisterMesh(m_Layout, vertices, indices, "Quad");
	}

	Shared<Mesh> PrimitiveMeshes::BuildPlane() const noexcept {
		constexpr float size = 5.0f;
		constexpr std::array<float, 4 * 8> vertices {
			-size,0,-size,  0,1,0,  0,0,
			 size,0,-size,  0,1,0,  1,0,
			 size,0, size,  0,1,0,  1,1,
			-size,0, size,  0,1,0,  0,1,
		 };

		constexpr std::array<Index, 6> indices { 0, 2, 1, 2, 0, 3 };

		return RegisterMesh(m_Layout, vertices, indices, "Plane");
	}

	Shared<Mesh> PrimitiveMeshes::BuildSphere() const noexcept {
	    constexpr int   rings   { 20 };
	    constexpr int   sectors { 20 };
	    constexpr float pi      { std::numbers::pi_v<float> };

	    std::vector<float> vertices{};
	    std::vector<Index> indices{};

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

	    return RegisterMesh(m_Layout, vertices, indices, "Sphere");
	}

	Shared<Mesh> PrimitiveMeshes::BuildCube() const noexcept {
	    constexpr std::array<float, 24 * 8> vertices {
	        // pos               normal    uv
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

		constexpr std::array<Index, 36> indices {
			0, 1, 2,  2, 3, 0,
			4, 5, 6,  6, 7, 4,
			8, 9, 10, 10, 11, 8,
			12, 13, 14, 14, 15, 12,
			16, 17, 18, 18, 19, 16,
			20, 21, 22, 22, 23, 20,
		};

	    return RegisterMesh(m_Layout, vertices, indices, "Cube");
	}

	Shared<Mesh> PrimitiveMeshes::BuildCapsule() const noexcept {
	    constexpr int   rings        	{ 10 };
	    constexpr int   sectors      	{ 20 };
	    constexpr float pi           	{ std::numbers::pi_v<float> };
	    constexpr float radius       	{ 0.5f };
	    constexpr float cylinderHeight  { 1.0f };

	    std::vector<float> vertices{};
	    std::vector<Index> indices{};

		constexpr int ringsPerHemisphere { rings };
		constexpr int totalRings { ringsPerHemisphere * 2 + 1 };

	    vertices.reserve(totalRings * (sectors + 1) * 8);

	    for (int i = 0; i <= ringsPerHemisphere; i++) {
	        const float v   { static_cast<float>(i) / ringsPerHemisphere };
	        const float phi { (v * pi) / 2.0f };

	        const float ringY = std::cos(phi) * radius + cylinderHeight * 0.5f;
	        const float ringR = std::sin(phi) * radius;

	        for (int j = 0; j <= sectors; j++) {
	            const float u     { static_cast<float>(j) / sectors };
	            const float theta { u * pi * 2.0f };

	            const float x { std::cos(theta) * ringR };
	            const float z { std::sin(theta) * ringR };

	            const float nx { std::cos(theta) * std::sin(phi) };
	            const float ny { std::cos(phi) };
	            const float nz { std::sin(theta) * std::sin(phi) };

	            vertices.push_back(x);
	            vertices.push_back(ringY);
	            vertices.push_back(z);
	            vertices.push_back(nx);
	            vertices.push_back(ny);
	            vertices.push_back(nz);
	            vertices.push_back(u);
	            vertices.push_back(v * 0.25f);
	        }
	    }

	    for (int i = 0; i <= ringsPerHemisphere; i++) {
	        const float v   { static_cast<float>(i) / ringsPerHemisphere };
	        const float phi { pi / 2.0f + (v * pi) / 2.0f };

	        const float ringY { std::cos(phi) * radius - cylinderHeight * 0.5f };
	        const float ringR { std::sin(phi) * radius };

	        for (int j = 0; j <= sectors; j++) {
	            const float u     { static_cast<float>(j) / sectors };
	            const float theta { u * pi * 2.0f };

	            const float x { std::cos(theta) * ringR };
	            const float z { std::sin(theta) * ringR };

	            const float nx { std::cos(theta) * std::sin(phi) };
	            const float ny { std::cos(phi) };
	            const float nz { std::sin(theta) * std::sin(phi) };

	            vertices.push_back(x);
	            vertices.push_back(ringY);
	            vertices.push_back(z);
	            vertices.push_back(nx);
	            vertices.push_back(ny);
	            vertices.push_back(nz);
	            vertices.push_back(u);
	            vertices.push_back(0.75f + v * 0.25f);
	        }
	    }

		constexpr int ringsCount { (ringsPerHemisphere + 1) * 2 };

	    for (int i = 0; i < ringsCount - 1; i++) {
	        for (int j = 0; j < sectors; j++) {
	            const Index first  { static_cast<Index>(i * (sectors + 1) + j) };
	            const Index second { first + sectors + 1 };

	            indices.insert(indices.end(), {
	                first, first + 1, second,
	                second, first + 1, second + 1
	            });
	        }
	    }

	    return RegisterMesh(m_Layout, vertices, indices, "Capsule");
	}

} // namespace Engine::Utils