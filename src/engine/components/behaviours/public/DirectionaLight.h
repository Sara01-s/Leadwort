#pragma once

#include "Behaviour.h"
#include "engine/systems/public/LightingSystem.h"
#include <engine/utils/public/Color.h>

namespace Engine::Components::Behaviours {

class DirectionalLight : public Behaviour {
public:
	float intensity = 1.0f;
	Utils::Color color = Utils::Color::White();

	DirectionalLight() = default;
	~DirectionalLight() override { DirectionalLight::OnDisable(); }

	void OnEnable() override {
		Systems::LightingSystem::Get().Register(this);
	}

	void OnDisable() override {
		Systems::LightingSystem::Get().Unregister();
	}
};

} // namespace Engine::Components::Behaviours