#pragma once

#include "engine/components/behaviours/public/Behaviour.h"
#include "engine/systems/public/LightingSystem.h"
#include "engine/core/math/public/Color.h"

namespace Engine::Components::Behaviours {

class DirectionalLight : public Behaviour {
public:
	float intensity = 5.0f;
	Color color = Color::White();

	void OnEnable() override {
		Systems::LightingSystem::Get().Register(this);
	}

	void OnDisable() override {
		Systems::LightingSystem::Get().Unregister();
	}

};

} // namespace Engine::Components::Behaviours