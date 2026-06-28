#pragma once

#include "engine/components/behaviours/public/Behaviour.h"
#include "engine/components/public/Transform.h"

#include "engine/core/math/public/Quat.h"
#include "engine/core/math/public/Vec3.h"
#include "engine/core/public/Time.h"
#include "engine/utils/public/Color.h"
#include "engine/systems/public/LightingSystem.h"

namespace Engine::Components::Behaviours {

class DirectionalLight : public Behaviour {
public:
	float intensity = 5.0f;
	Utils::Color color = Utils::Color::White();

	void OnEnable() override {
		Systems::LightingSystem::Get().Register(this);
	}

	void OnDisable() override {
		Systems::LightingSystem::Get().Unregister();
	}

};

} // namespace Engine::Components::Behaviours