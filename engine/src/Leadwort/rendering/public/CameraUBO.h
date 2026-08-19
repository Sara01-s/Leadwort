#pragma once

#include "Leadwort/core/math/public/Mat4.h"

#include <glad/glad.h>

namespace Leadwort::Components { class Camera; }

namespace Leadwort::Rendering {

	class CameraUBO {
	public:
		struct alignas(16) CameraDataGPU {
			Mat4 view;           // 64 bytes
			Mat4 projection;     // 64 bytes
			Vec4 cameraPosition; // 16 bytes
			Vec4 padding;        // 16 bytes
		};

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