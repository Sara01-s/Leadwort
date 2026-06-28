#pragma once

#include <glad/glad.h>

namespace Engine::Components::Behaviours { class DirectionalLight; }

namespace Engine::Rendering {

// std140 layout — 32 bytes total:
//
// offset  0 → vec4 direction  (12 bytes + 4 padding)
// offset 16 → vec4 color      (12 bytes + 4 padding)
// offset 28 → float intensity (4 bytes, packed into color's padding)
//

class LightingUBO {
public:
	void Initialize();
	void Update(const Components::Behaviours::DirectionalLight* light) const;

	~LightingUBO();

private:
	GLuint m_UBO { 0 };
};

} // namespace Engine::Rendering