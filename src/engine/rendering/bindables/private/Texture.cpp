#include "../public/Texture.h"

#include "engine/asset-management/private/AssetKey.h"
#include "engine/asset-management/public/AssetManager.h"
#include "engine/utils/public/Logger.h"

#include <GL/glext.h>
#include <glad/glad.h>

namespace Engine::Rendering::Bindables {

namespace {

float GetMaxAnisotropy() {
	float value = 1.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &value);
	return value;
}

} // namespace

Texture::Texture(AssetManagement::AssetKey<Texture>) {}

Texture::~Texture() {
	if (m_GpuID != 0) {
		CORE_LOG("Destroying Texture with GPU ID: ", m_GpuID);
		glDeleteTextures(1, &m_GpuID);
		m_GpuID = 0;
	}
}

void Texture::ApplySamplerParams(const bool generateMipmaps, const bool anisotropicFiltering) {
	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (anisotropicFiltering) {
		glTexParameterf(
			GL_TEXTURE_2D,
			GL_TEXTURE_MAX_ANISOTROPY_EXT,
			GetMaxAnisotropy()
		);
	}
}

void Texture::UploadRGBA(
	const uint8_t* pixels,
	const int width,
	const int height,
	const bool generateMipmaps,
	const bool anisotropicFiltering
) {
	m_Width    = width;
	m_Height   = height;
	m_Channels = 4;

	if (m_GpuID == 0) {
		glGenTextures(1, &m_GpuID);
	}

	glBindTexture(GL_TEXTURE_2D, m_GpuID);

	ApplySamplerParams(generateMipmaps, anisotropicFiltering);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA8,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		pixels
	);

	if (generateMipmaps) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}
}

void Texture::Bind(const int slot) const {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_GpuID);
}

void Texture::Bind() const noexcept {
	Bind(0);
}

void Texture::Unbind() const noexcept {
	glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace Engine::Rendering::Bindables