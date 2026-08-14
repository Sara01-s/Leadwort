#include "../public/Texture.h"

#include "tinyexr/tinyexr.h"

#include <Leadwort/asset-management/private/AssetKey.h>
#include <Leadwort/asset-management/public/AssetDatabase.h>
#include <Leadwort/utils/public/Logger.h>

#include <GL/glext.h>
#include <glad/glad.h>

namespace Leadwort::Rendering::Bindables {

	float GetMaxAnisotropy() {
		float value { 1.0f };
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &value);
		return value;
	}

	Texture::Texture(AssetManagement::AssetKey<Texture>, const std::string_view name) : m_Name(name) {}

	Texture::Texture(std::string_view exrPath, AssetManagement::AssetKey<Texture>) {
		float* exrData { nullptr };
		const char* error { nullptr };

		const int ret { LoadEXR(&exrData, &m_Width, &m_Height, exrPath.data(), &error) };
		if (ret != TINYEXR_SUCCESS) {
			const std::string errorMsg { error ? error : "Unknown error" };

			if (error) {
				FreeEXRErrorMessage(error);
			}

			LW_ERROR("Error loading EXR: ", errorMsg, " path: ", exrPath);
		}

		m_Name = std::filesystem::path(exrPath).stem().string();
		m_Path = std::string(exrPath);

		m_GpuID = 0;
		glGenTextures(1, &m_GpuID);
		glBindTexture(GL_TEXTURE_2D, m_GpuID);

		m_Channels = 4;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, exrData);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		free(exrData);
	}

	Texture::~Texture() {
		if (m_GpuID != 0) {
			LW_LOG("Destroying Texture with GPU ID: ", m_GpuID);
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

}