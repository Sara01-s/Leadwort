#pragma once

#include <Leadwort/rendering/bindables/public/Mesh.h>
#include <Leadwort/utils/public/Singleton.h>

namespace Leadwort::Utils {

class PrimitiveMeshes : public Singleton<PrimitiveMeshes> {
	friend class Singleton;
public:
	Shared<Rendering::Bindables::Mesh> Empty();
	Shared<Rendering::Bindables::Mesh> Quad();
	Shared<Rendering::Bindables::Mesh> Plane();
	Shared<Rendering::Bindables::Mesh> Sphere();
	Shared<Rendering::Bindables::Mesh> Cube();
	Shared<Rendering::Bindables::Mesh> Capsule();

private:
	PrimitiveMeshes();

	Shared<Rendering::Bindables::Mesh> BuildEmpty()   const noexcept;
	Shared<Rendering::Bindables::Mesh> BuildQuad()    const noexcept;
	Shared<Rendering::Bindables::Mesh> BuildPlane()   const noexcept;
	Shared<Rendering::Bindables::Mesh> BuildSphere()  const noexcept;
	Shared<Rendering::Bindables::Mesh> BuildCube()    const noexcept;
	Shared<Rendering::Bindables::Mesh> BuildCapsule() const noexcept;

	Rendering::Bindables::VertexLayout m_Layout{};

	Shared<Rendering::Bindables::Mesh> m_Empty;
	Shared<Rendering::Bindables::Mesh> m_Quad;
	Shared<Rendering::Bindables::Mesh> m_Plane;
	Shared<Rendering::Bindables::Mesh> m_Sphere;
	Shared<Rendering::Bindables::Mesh> m_Cube;
	Shared<Rendering::Bindables::Mesh> m_Capsule{};
};

} // namespace Engine::Utils