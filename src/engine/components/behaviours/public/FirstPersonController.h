#pragma once

#include "engine/components/behaviours/public/Behaviour.h"
#include "engine/components/public/Transform.h"
#include "engine/core/public/Time.h"
#include "engine/systems/public/Input.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <numbers>

namespace Engine::Components::Behaviours {

class FirstPersonController final : public Behaviour {
public:
    float moveSpeed       = 8.0f;
    float lookSensitivity = 0.15f;

    void Start() override {
        Systems::Input::Mouse::SetCaptured(true);

        const auto fwd = GetTransform()->GetForward();
        m_Yaw   = glm::degrees(std::atan2(fwd.x, fwd.z));
        m_Pitch = glm::degrees(std::asin(-fwd.y));
    }

    void Update() override {
        if (Systems::Input::Keyboard::IsJustPressed(Systems::Key::Escape)) {
            Systems::Input::Mouse::SetCaptured(!Systems::Input::Mouse::IsCaptured());
        }

        if (!Systems::Input::Mouse::IsCaptured()) {
            return;
        }

        // Mouse rotation
        const auto mouseDelta = Systems::Input::Mouse::GetDelta();
        m_Yaw   += mouseDelta.x * lookSensitivity;
        m_Pitch += mouseDelta.y * lookSensitivity;
        m_Pitch  = glm::clamp(m_Pitch, -89.9f, 89.9f);

        constexpr float toRad = std::numbers::pi_v<float> / 180.0f;
        const auto qYaw   = glm::angleAxis(m_Yaw   * toRad, glm::vec3(0.0f, 1.0f, 0.0f));
        const auto qPitch = glm::angleAxis(m_Pitch * toRad, glm::vec3(1.0f, 0.0f, 0.0f));
        GetTransform()->SetLocalRotation(qYaw * qPitch);

        // Movement
		auto moveDirection = glm::vec3(0.0f);

        const float horizontal = Systems::Input::GetAxis(Systems::Player::P1, Systems::Axis::Horizontal);
        const float vertical   = Systems::Input::GetAxis(Systems::Player::P1, Systems::Axis::Vertical);

        const auto fwd         = GetTransform()->GetForward();
        const auto forwardFlat = glm::normalize(glm::vec3(fwd.x, 0.0f, fwd.z));
        const auto rightFlat   = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), forwardFlat));

        if (horizontal != 0.0f) moveDirection = moveDirection + rightFlat   * horizontal;
        if (vertical   != 0.0f) moveDirection = moveDirection + forwardFlat * vertical;

        if (Systems::Input::Keyboard::IsPressed(Systems::Key::Space))     moveDirection = moveDirection + glm::vec3(0.0f,  1.0f, 0.0f);
        if (Systems::Input::Keyboard::IsPressed(Systems::Key::LeftShift)) moveDirection = moveDirection + glm::vec3(0.0f, -1.0f, 0.0f);

        if (glm::dot(moveDirection, moveDirection) > 0.0001f) {
            const auto velocity = glm::normalize(moveDirection) * moveSpeed * Core::Time::GetDeltaTime();
            GetTransform()->SetWorldPosition(GetTransform()->GetWorldPosition() + velocity);
        }
    }

private:
    float m_Pitch = 0.0f;
    float m_Yaw   = 0.0f;
};

} // namespace Engine::Components::Behaviours