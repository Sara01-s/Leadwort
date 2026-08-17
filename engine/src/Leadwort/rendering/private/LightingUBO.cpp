#include <Leadwort/rendering/public/LightingUBO.h>

#include "Leadwort/components/public/Transform.h"

#include <Leadwort/components/behaviours/public/Light.h>
#include <Leadwort/core/public/Entity.h>
#include <cmath>

namespace Leadwort::Rendering {

	void LightingUBO::Initialize() {
		glGenBuffers(1, &m_UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
		glBufferData(GL_UNIFORM_BUFFER, LIGHTING_UBO_SIZE_BYTES, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, LIGHTING_UBO_BINDING, m_UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	LightingUBO::~LightingUBO() {
		if (m_UBO) {
			glDeleteBuffers(1, &m_UBO);
		}
	}

	void LightingUBO::Update(const std::array<Components::Behaviours::Light*, MAX_LIGHTS>& lights) const {
		LightingDataGPU data;

		int pointIndex { 0 };
		int spotIndex { 0 };

		for (const auto* light: lights) {
			if (!light) {
				continue;
			}

			const auto& transform = light->GetEntity().GetTransform();
			const Vec3 pos = transform.GetLocalPosition();
			const Vec3 dir = transform.GetForward();

			switch (light->Type) {
				case Components::Behaviours::Light::LightType::Directional: {
					data.Direction = Vec4(dir.x, dir.y, dir.z, 0.0f);
					data.ColorIntensity = Vec4(light->Color.r, light->Color.g, light->Color.b, light->Intensity);
					break;
				}
				case Components::Behaviours::Light::LightType::Point: {
					if (pointIndex < MAX_POINT_LIGHTS) {
						auto& p = data.PointLights[pointIndex++];
						p.Position = Vec4(pos.x, pos.y, pos.z, 1.0f);
						p.Color = Vec4(light->Color.r, light->Color.g, light->Color.b, light->Intensity);
						p.Attenuation = Vec4(light->Attenuation.x, light->Attenuation.y, light->Attenuation.z, 1.0f);
					}
					break;
				}
				case Components::Behaviours::Light::LightType::Spot: {
					if (spotIndex < MAX_SPOT_LIGHTS) {
						auto& s = data.SpotLights[spotIndex++];
						s.Position = Vec4(pos.x, pos.y, pos.z, 1.0f);
						s.Direction = Vec4(dir.x, dir.y, dir.z, 0.0f);
						s.Color = Vec4(light->Color.r, light->Color.g, light->Color.b, light->Intensity);
						s.Attenuation = Vec4(light->Attenuation.x, light->Attenuation.y, light->Attenuation.z, 1.0f);
						s.Cutoffs = Vec4(
							std::cos(light->InnerCutoff * DegToRad),
							std::cos(light->OuterCutoff * DegToRad),
							0.0f, 0.0f
						);
					}
					break;
				}
			}
		}

		data.LightCounts[0] = pointIndex;
		data.LightCounts[1] = spotIndex;

		glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, LIGHTING_UBO_SIZE_BYTES, &data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

} // namespace Leadwort::Rendering