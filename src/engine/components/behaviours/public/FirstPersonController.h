#pragma once

#include "engine/components/behaviours/public/Behaviour.h"
#include "engine/components/public/Transform.h"
#include "engine/core/public/Time.h"
#include "engine/systems/public/Input.h"

namespace Engine::Components::Behaviours {

class FirstPersonController final : public Behaviour {
public:
    float moveSpeed       = 20.0f;
    float lookSensitivity = 0.15f;

    void Start() override {
        Systems::Input::Mouse::SetCaptured(true);
        m_Rotation = GetTransform().GetLocalRotation();
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
        const float yawDelta   = mouseDelta.x * lookSensitivity;
        const float pitchDelta = mouseDelta.y * lookSensitivity;

        const Quat qYaw   = Quat::FromAngleAxis(yawDelta,   Vec3::Up());
        const Quat qPitch = Quat::FromAngleAxis(pitchDelta, Vec3::Right());

        m_Rotation = (qYaw * m_Rotation * qPitch).Normalized();
        GetTransform().SetLocalRotation(m_Rotation);

        // Translation
        auto moveDirection = Vec3::Zero();
        const float horizontal = Systems::Input::GetAxis(Systems::Player::P1, Systems::Axis::Horizontal);
        const float vertical   = Systems::Input::GetAxis(Systems::Player::P1, Systems::Axis::Vertical);

        const Vec3 forward = GetTransform().GetForward();
        const Vec3 right = GetTransform().GetRight();

        const Vec3 forwardFlat = Vec3(forward.x, 0.0f, forward.z).Normalized();
        const Vec3 rightFlat   = Vec3(right.x, 0.0f, right.z).Normalized();

        moveDirection += forwardFlat * vertical;
        moveDirection += rightFlat   * horizontal;

        if (Systems::Input::Keyboard::IsPressed(Systems::Key::Space))     moveDirection.y += 1.0f;
        if (Systems::Input::Keyboard::IsPressed(Systems::Key::LeftShift)) moveDirection.y -= 1.0f;

        if (moveDirection.LengthSquared() > 0.0001f) {
           const auto velocity = moveDirection.Normalized() * moveSpeed * Core::Time::GetDeltaTime();
           GetTransform().SetWorldPosition(GetTransform().GetWorldPosition() + velocity);
        }
    }

private:
    Quat m_Rotation = Quat::Identity();
};

} // namespace Engine::Components::Behaviours