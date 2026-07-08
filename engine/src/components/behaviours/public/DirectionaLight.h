#pragma once

#include "components/behaviours/public/Behaviour.h"
#include "systems/public/LightingSystem.h"
#include "core/math/public/Color.h"

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