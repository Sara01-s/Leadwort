#pragma once

#include <glad/glad.h>

namespace Leadwort::Rendering {

class TimeUBO {
public:
	TimeUBO() = default;
	~TimeUBO();

	void Initialize();
	TimeUBO(const TimeUBO&) = delete;
	TimeUBO& operator=(const TimeUBO&) = delete;

	// x = time, y = time / 2, z = time * 2, w = delta time
	void Update(float time, float deltaTime) const;

private:
	GLuint m_UBO { 0 };
};

} // namespace Engine::Rendering