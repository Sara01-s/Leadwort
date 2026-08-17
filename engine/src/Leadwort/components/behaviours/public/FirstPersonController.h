#pragma once

#include <Leadwort/components/behaviours/public/Behaviour.h>
#include <Leadwort/components/public/Transform.h>
#include <Leadwort/core/public/Time.h>
#include <Leadwort/systems/public/Input.h>

namespace Leadwort::Components::Behaviours {

	class FirstPersonController final : public Behaviour {
		using Input = Systems::Input;
	public:
		LW_REFLECT(FirstPersonController,
			LW_FIELD(Float, moveSpeed, "Move Speed"),
			LW_FIELD(Float, lookSensitivity, "Sensitivity")
		)

	    float moveSpeed       { 20.0f };
	    float lookSensitivity { 0.15f };

	    void Start() override {
	        m_Rotation = GetTransform().GetLocalRotation();
	    }

	    void Update() override {
    		if (Input::Mouse::IsButtonJustPressed(Input::Mouse::Button::Right) && Input::Mouse::IsViewportHovered()) {
    			Input::Mouse::SetCaptured(true);
    		}

    		if (Input::Mouse::IsButtonJustReleased(Input::Mouse::Button::Right) && Input::Mouse::IsCaptured()) {
    			Input::Mouse::SetCaptured(false);
    		}

    		if (!Input::Mouse::IsCaptured()) {
    			return;
    		}

	        // Mouse rotation
	        const auto mouseDelta  { Input::Mouse::GetDelta() };
	        const float yawDelta   { mouseDelta.x * lookSensitivity };
	        const float pitchDelta { mouseDelta.y * lookSensitivity };

	        const Quat qYaw   { Quat::FromAngleAxis(yawDelta,   Vec3::Up()) };
	        const Quat qPitch { Quat::FromAngleAxis(pitchDelta, Vec3::Right()) };

	        m_Rotation = (qYaw * m_Rotation * qPitch).Normalized();
	        GetTransform().SetLocalRotation(m_Rotation);

	        // Translation
	        auto moveDirection { Vec3::Zero() };
	        const float horizontal { Input::GetAxis(Systems::Player::P1, Systems::Axis::Horizontal) };
	        const float vertical   { Input::GetAxis(Systems::Player::P1, Systems::Axis::Vertical) };

	        const Vec3 forward { GetTransform().GetForward() };
	        const Vec3 right { GetTransform().GetRight() };

	        const Vec3 forwardFlat { Vec3(forward.x, 0.0f, forward.z).Normalized() };
	        const Vec3 rightFlat   { Vec3(right.x, 0.0f, right.z).Normalized() };

	        moveDirection += forwardFlat * vertical;
	        moveDirection += rightFlat   * horizontal;

	        if (Input::Keyboard::IsPressed(Systems::Key::E)) moveDirection.y += 1.0f;
	        if (Input::Keyboard::IsPressed(Systems::Key::Q)) moveDirection.y -= 1.0f;

	        if (moveDirection.LengthSquared() > 0.0001f) {
	           const auto velocity = moveDirection.Normalized() * moveSpeed * Core::Time::GetDeltaTime();
	           GetTransform().SetWorldPosition(GetTransform().GetWorldPosition() + velocity);
	        }
	    }

	private:
	    Quat m_Rotation = Quat::Identity();
	};

	LW_REGISTER_COMPONENT(FirstPersonController)

} // namespace Engine::Components::Behaviours