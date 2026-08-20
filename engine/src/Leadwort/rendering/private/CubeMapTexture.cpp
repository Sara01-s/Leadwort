#include "Leadwort/rendering/CubeMapTexture.h"

#include <cmath>

namespace Leadwort::Rendering {

	CubeMapTexture::CubeMapTexture(const int resolution, const Format format, const bool generateMips)
		: m_Resolution(resolution), m_Format(format)
	{
		m_MipLevels = generateMips
			? static_cast<int>(std::floor(std::log2(resolution))) + 1
			: 1;

		glGenTextures(1, &m_TextureGpuID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureGpuID);

		for (int face = 0; face < 6; face++) {
			for (int mip = 0; mip < m_MipLevels; mip++) {
				const int mipRes = resolution >> mip;
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip,
					GL_RGB16F, mipRes, mipRes, 0,
					GL_RGB, GL_FLOAT, nullptr
				);
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, generateMips ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	CubeMapTexture::~CubeMapTexture() {
		if (m_TextureGpuID != 0) {
			glDeleteTextures(1, &m_TextureGpuID);
		}
	}

	void CubeMapTexture::BindAsInput(const GLint slot) const noexcept {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureGpuID);
	}

}