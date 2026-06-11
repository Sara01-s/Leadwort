#pragma once

#include "engine/rendering/bindables/public/Mesh.h"
#include "engine/utils/public/Singleton.h"

namespace Engine::Utils {

class PrimitiveMeshes : public Singleton<PrimitiveMeshes> {
	friend class Singleton;
public:
	Shared<Rendering::Bindables::Mesh> Quad();
	Shared<Rendering::Bindables::Mesh> Plane();
	Shared<Rendering::Bindables::Mesh> Sphere();
	Shared<Rendering::Bindables::Mesh> Cube();

private:
	PrimitiveMeshes();

	Shared<Rendering::Bindables::Mesh> BuildQuad()   const;
	Shared<Rendering::Bindables::Mesh> BuildPlane()  const;
	Shared<Rendering::Bindables::Mesh> BuildSphere() const;
	Shared<Rendering::Bindables::Mesh> BuildCube()   const;

	Rendering::Bindables::VertexLayout m_Layout{};

	Shared<Rendering::Bindables::Mesh> m_Quad;
	Shared<Rendering::Bindables::Mesh> m_Plane;
	Shared<Rendering::Bindables::Mesh> m_Sphere;
	Shared<Rendering::Bindables::Mesh> m_Cube;
};

} // namespace Engine::Utils