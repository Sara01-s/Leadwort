#pragma once

#include "Leadwort/core/public/Core.h"

#include <glad/glad.h>

namespace Leadwort::Components { class Camera; }

namespace Leadwort::Rendering {

	class CameraUBO {
	public:
		CameraUBO() = default;
	    ~CameraUBO();

	    void Initialize();
	    CameraUBO(const CameraUBO&) = delete;
	    CameraUBO& operator=(const CameraUBO&) = delete;

	    void Update(const Components::Camera& camera) const;

	private:
	    GLuint m_UBO { 0 };
	};

} // namespace Engine::Rendering