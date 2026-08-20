#type vertex
#version 450 core

layout(location = 0) in vec3 a_position;

uniform mat4 _ViewMatrix;
uniform mat4 _ProjectionMatrix;

out vec3 v_localPos;

void main() {
	v_localPos = a_position;
	gl_Position = _ProjectionMatrix * _ViewMatrix * vec4(a_position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;

uniform samplerCube _EnvironmentMap;

in vec3 v_localPos;

const float PI = 3.14159265359;

void main() {
	vec3 normal = normalize(v_localPos);

	vec3 irradiance = vec3(0.0);

	// orthonormal basis to sample interior faces
	vec3 up    = abs(normal.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
	vec3 right = normalize(cross(up, normal));
	up         = normalize(cross(normal, right));

	const float sampleDelta = 0.025;
	float sampleCount = 0.0;

	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

			irradiance += texture(_EnvironmentMap, sampleVec).rgb * cos(theta) * sin(theta);
			sampleCount++;
		}
	}

	irradiance = PI * irradiance / sampleCount;
	fragColor = vec4(irradiance, 1.0);
}