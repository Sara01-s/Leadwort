#include "../public/Texture.h"

#include "engine/asset-management/public/AssetManager.h"

#include <GL/glext.h>
#include <glad/glad.h>
#include <stb/stb_image.h>

namespace Engine::Rendering::Bindables {

static float GetMaxAnisotropy() {
    float v = 1.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &v);
    return v;
}

void Texture::ApplySamplerParams(const bool mips, const bool aniso) {
    const int minFilter = mips ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    if (aniso) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, GetMaxAnisotropy());
    }
}

// Uploads RGBA pixel data and optionally generates mipmaps.
void Texture::UploadRGBA(const uint8_t* pixels, const int width, const int height, const bool mips, const bool aniso) {
    m_Width    = width;
    m_Height   = height;
    m_Channels = 4;

    glGenTextures(1, &m_GpuID);
    glBindTexture(GL_TEXTURE_2D, m_GpuID);

    ApplySamplerParams(mips, aniso);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    if (mips) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}

// ─────────────────────────────────────────────
//  Factory methods
// ─────────────────────────────────────────────

Texture* Texture::FromPath(const std::string& path) {
    auto* tex = new Texture();
    tex->LoadFromPath(path);
    return tex;
}

Texture* Texture::FromMemory(const uint8_t* data, const size_t size) {
    int w, h, c;
    stbi_set_flip_vertically_on_load(true);
    uint8_t* pixels = stbi_load_from_memory(data, static_cast<int>(size), &w, &h, &c, STBI_rgb_alpha);

    if (!pixels) {
        throw std::runtime_error(std::string("Texture::FromMemory failed: ") + stbi_failure_reason());
    }

    auto* tex = new Texture();
    tex->UploadRGBA(pixels, w, h, /*mips=*/true, /*aniso=*/true);
    stbi_image_free(pixels);
    return tex;
}

Texture* Texture::FromRGBA(const int width, const int height, const uint8_t* rgbaPixels) {
    auto* tex = new Texture();
    tex->UploadRGBA(rgbaPixels, width, height, /*mips=*/true, /*aniso=*/true);
    return tex;
}

Texture* Texture::CreateSolid(const Utils::Color& color) {
    const uint8_t pixel[4] = {
        static_cast<uint8_t>(color.r * 255.0f),
        static_cast<uint8_t>(color.g * 255.0f),
        static_cast<uint8_t>(color.b * 255.0f),
        static_cast<uint8_t>(color.a * 255.0f),
    };

    auto* tex = new Texture();
    tex->m_Width    = 1;
    tex->m_Height   = 1;
    tex->m_Channels = 4;

    glGenTextures(1, &tex->m_GpuID);
    glBindTexture(GL_TEXTURE_2D, tex->m_GpuID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    return tex;
}

Texture* Texture::CreateEmpty() {
    auto* tex = new Texture();
    glGenTextures(1, &tex->m_GpuID);
    return tex;
}

// ─────────────────────────────────────────────
//  Private loading
// ─────────────────────────────────────────────

void Texture::LoadFromPath(const std::string& path) {
    stbi_set_flip_vertically_on_load(true);

    const std::vector<uint8_t> bytes = AssetManagement::EngineAssets::LoadBytes(path);

    int w, h, c;
    uint8_t* pixels = stbi_load_from_memory(bytes.data(), static_cast<int>(bytes.size()), &w, &h, &c, STBI_rgb_alpha);

    if (!pixels) {
        throw std::runtime_error("Failed to load texture '" + path + "': " + stbi_failure_reason());
    }

    UploadRGBA(pixels, w, h, /*mips=*/true, /*aniso=*/true);
    stbi_image_free(pixels);
}

// ─────────────────────────────────────────────
//  Bind / Unbind / Destroy
// ─────────────────────────────────────────────

Texture::~Texture() {
    if (m_GpuID != 0) {
        glDeleteTextures(1, &m_GpuID);
    }
}

void Texture::Bind(const int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_GpuID);
}

void Texture::Bind() const noexcept   { Bind(0); }
void Texture::Unbind() const noexcept { glBindTexture(GL_TEXTURE_2D, 0); }

} // namespace Engine::Rendering::Bindables