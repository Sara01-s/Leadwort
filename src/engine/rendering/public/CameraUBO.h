#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Engine::Components { class Camera; }

namespace Engine::Rendering {

class CameraUBO {
public:
	CameraUBO() = default;
    ~CameraUBO();

    void Init();
    CameraUBO(const CameraUBO&) = delete;
    CameraUBO& operator=(const CameraUBO&) = delete;

    void Update(const Components::Camera* camera) const;

private:
    GLuint m_UBO = 0;
};

} // namespace Engine::Rendering