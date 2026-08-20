#pragma once

#include "Leadwort/core/math/public/IntVec4.h"
#include "Leadwort/core/math/public/Mat4.h"
#include "Leadwort/core/math/public/Vec4.h"

#include <glad/glad.h>
#include <array>

namespace Leadwort::Components::Behaviours { class Light; }

namespace Leadwort::Rendering {

	static constexpr int MAX_POINT_LIGHTS { 8 };
	static constexpr int MAX_SPOT_LIGHTS { 8 };
	// (+1) because that accounts for 1 directional light
	static constexpr int MAX_LIGHTS { MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS + 1 };

	struct alignas(16) PointLightGPU {
		Vec4 Position;
		Vec4 Color;       // rgb = color, a = intensity
		Vec4 Attenuation; // xyz = attenuation
	};

	struct alignas(16) SpotLightGPU {
		Vec4 Position;
		Vec4 Direction;
		Vec4 Color;       // rgb = color, a = intensity
		Vec4 Attenuation;
		Vec4 Cutoffs;     // x = cos(inner), y = cos(outer)
	};

	// Must match shd_lighting.glsl LightingData declaration exactly, field order included.
	struct alignas(16) LightingDataGPU {
		Vec4 Direction{};
		Vec4 ColorIntensity{};
		std::array<PointLightGPU, MAX_POINT_LIGHTS> PointLights;
		std::array<SpotLightGPU, MAX_SPOT_LIGHTS> SpotLights;
		Mat4 LightSpaceMatrix{};
		IntVec4 LightCounts{};
	};

	class LightingUBO {
	public:
		void Initialize();
		void Update(const std::array<Components::Behaviours::Light*, MAX_LIGHTS>& lights, const Mat4& lightSpaceMatrix) const;

		~LightingUBO();

	private:
		GLuint m_UBO { 0 };
	};

}