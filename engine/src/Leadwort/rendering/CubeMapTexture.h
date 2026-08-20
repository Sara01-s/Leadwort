#pragma once
#include <glad/glad.h>

namespace Leadwort::Rendering {

class CubeMapTexture {
public:
	enum class Format {
		RGB16F, // HDR color, env cubemap / irradiance / prefilter
	};

	CubeMapTexture(int resolution, Format format, bool generateMips = false);
	~CubeMapTexture();

	CubeMapTexture(const CubeMapTexture&) = delete;
	CubeMapTexture& operator=(const CubeMapTexture&) = delete;

	void BindAsInput(GLint slot) const noexcept;
	[[nodiscard]] GLuint GetGpuID() const noexcept { return m_TextureGpuID; }
	[[nodiscard]] int GetResolution() const noexcept { return m_Resolution; }
	[[nodiscard]] int GetMipLevels() const noexcept { return m_MipLevels; }

private:
	int    m_Resolution{};
	Format m_Format{};
	int    m_MipLevels{ 1 };
	GLuint m_TextureGpuID{ 0 };
};

}