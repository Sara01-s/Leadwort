#include "../public/CubeMap.h"

#include "engine/asset-management/private/AssetKey.h"

#include <glad/glad.h>
#include <stb_image.h>
#include <stdexcept>

namespace Engine::Rendering::Bindables {

CubeMap::CubeMap(const std::array<std::string, 6>& paths, AssetManagement::AssetKey<CubeMap>) {
    m_GpuID = 0;
    glGenTextures(1, &m_GpuID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_GpuID);

    stbi_set_flip_vertically_on_load(false);

    for (int i = 0; i < 6; ++i) {
        int width, height, channels;
        unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &channels, 4);

        if (data == nullptr) {
            throw std::runtime_error("Error while loading face: " + paths[i] + " - " + stbi_failure_reason());
        }

        glTexImage2D(
            /* target         = */ GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            /* level          = */ 0,
            /* internalformat = */ GL_RGBA,
            /* width          = */ width,
            /* height         = */ height,
            /* border         = */ 0,
            /* format         = */ GL_RGBA,
            /* type           = */ GL_UNSIGNED_BYTE,
            /* pixels         = */ data
        );

        stbi_image_free(data);
    }

    stbi_set_flip_vertically_on_load(true);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

CubeMap::~CubeMap() {
    Unbind();
    glDeleteTextures(1, &m_GpuID);
}

void CubeMap::Bind(const int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_GpuID);
}

void CubeMap::Bind()   const noexcept { Bind(0); }
void CubeMap::Unbind() const noexcept { glBindTexture(GL_TEXTURE_CUBE_MAP, 0); }

} // namespace Engine::Rendering::Bindables