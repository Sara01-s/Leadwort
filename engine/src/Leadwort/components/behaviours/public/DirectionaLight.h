#pragma once

#include <Leadwort/components/behaviours/public/Behaviour.h>
#include <Leadwort/systems/public/LightingSystem.h>
#include <Leadwort/core/math/public/Color.h>

namespace Leadwort::Components::Behaviours {

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