#pragma once

#include "engine/rendering/bindables/public/Mesh.h"
#include "engine/utils/public/Singleton.h"

#include <memory>

namespace Engine::Utils {

class PrimitiveMeshes : public Singleton<PrimitiveMeshes> {
	friend class Singleton;
public:
	Rendering::Bindables::Mesh* Empty();
	Rendering::Bindables::Mesh* Quad();
	Rendering::Bindables::Mesh* Plane();
	Rendering::Bindables::Mesh* Sphere();
	Rendering::Bindables::Mesh* Cube();

private:
	PrimitiveMeshes();

	std::unique_ptr<Rendering::Bindables::Mesh> BuildEmpty()  const;
	std::unique_ptr<Rendering::Bindables::Mesh> BuildQuad()   const;
	std::unique_ptr<Rendering::Bindables::Mesh> BuildPlane()  const;
	std::unique_ptr<Rendering::Bindables::Mesh> BuildSphere() const;
	std::unique_ptr<Rendering::Bindables::Mesh> BuildCube()   const;

	Rendering::Bindables::VertexLayout m_Layout{};
	std::shared_ptr<Rendering::Bindables::Material> m_Material;

	std::unique_ptr<Rendering::Bindables::Mesh> m_Empty;
	std::unique_ptr<Rendering::Bindables::Mesh> m_Quad;
	std::unique_ptr<Rendering::Bindables::Mesh> m_Plane;
	std::unique_ptr<Rendering::Bindables::Mesh> m_Sphere;
	std::unique_ptr<Rendering::Bindables::Mesh> m_Cube;
};

} // namespace Engine::Utils