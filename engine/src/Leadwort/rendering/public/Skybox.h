#pragma once

#include <Leadwort/rendering/bindables/public/Material.h>
#include <Leadwort/core/math/public/Vec3.h>
#include <Leadwort/core/math/public/Vec4.h>

#include <string>

namespace Leadwort::Rendering {

class Skybox {
public:
	explicit Skybox(std::string_view exrPath);
	~Skybox();

	Skybox(const Skybox&) = delete;
	Skybox& operator=(const Skybox&) = delete;

	void Render() const;

	[[nodiscard]] float GetRotation() const noexcept { return m_RotationDegrees; }
	void SetRotation(float degrees) noexcept;

	[[nodiscard]] Vec3 GetTint() const noexcept { return m_Tint; }
	void SetTint(const Vec3& tint) noexcept;

	[[nodiscard]] float GetExposure() const noexcept { return m_Exposure; }
	void SetExposure(float exposure) noexcept;

private:
	GpuID m_Vao { 0 };
	GpuID m_Vbo { 0 };
	GpuID m_Ibo { 0 };

	Shared<Bindables::Material> m_SkyboxMaterial{};

	float m_RotationDegrees { 0.0f };
	Vec3  m_Tint            { 1.0f, 1.0f, 1.0f };
	float m_Exposure        { 1.0f };
};

}