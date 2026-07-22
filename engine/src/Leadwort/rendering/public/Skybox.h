#pragma once

#include <Leadwort/rendering/bindables/public/Material.h>

#include <string>

namespace Leadwort::Rendering {

class Skybox {
public:
	explicit Skybox(std::string_view exrPath);
	~Skybox();

	Skybox(const Skybox&) = delete;
	Skybox& operator=(const Skybox&) = delete;

	void Render() const;

private:
	GpuID m_Vao { 0 };
	GpuID m_Vbo { 0 };
	GpuID m_Ibo { 0 };

	Shared<Bindables::Material> m_SkyboxMaterial{};
};

}