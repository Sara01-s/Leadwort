#pragma once
#include "Leadwort/components/ComponentRegistry.h"

#include <Leadwort/components/Component.h>
#include <Leadwort/core/math/public/Mat4.h>
#include <Leadwort/core/math/public/Math.h>
#include <Leadwort/core/math/public/Quat.h>
#include <Leadwort/core/math/public/Vec3.h>
#include <Leadwort/utils/public/Logger.h>

#include <vector>

namespace Leadwort::Components {

	class Transform : public Component {
	public:
		LW_REFLECT(Transform,
			LW_FIELD(Vec3, m_LocalPosition, "Position"),
			LW_FIELD(Quat, m_LocalRotation, "Rotation"),
			LW_FIELD(Vec3, m_LocalScale, "Scale")
		)

		// Local space
		void SetLocalPosition(const Vec3& position);
		void SetLocalRotation(const Quat& rotation);
		void SetLocalScale(const Vec3& scale);

		[[nodiscard]] const Vec3& GetLocalPosition() const { return m_LocalPosition; }
		[[nodiscard]] const Quat& GetLocalRotation() const { return m_LocalRotation; }
		[[nodiscard]] const Vec3& GetLocalScale()    const { return m_LocalScale; }

		[[nodiscard]] Vec3 GetForward() const { return GetWorldRotation() * Vec3::Forward(); }
		[[nodiscard]] Vec3 GetRight()   const { return GetWorldRotation() * Vec3::Right(); }
		[[nodiscard]] Vec3 GetUp()      const { return GetWorldRotation() * Vec3::Up(); }

		// World space
		[[nodiscard]] Vec3 GetWorldPosition() const;
		[[nodiscard]] Quat GetWorldRotation() const;
		[[nodiscard]] Vec3 GetWorldScale() const;

		void SetWorldPosition(const Vec3& worldPosition);
		void SetWorldRotation(const Quat& worldRotation);
		void SetWorldScale(const Vec3& worldScale);

		// Matrices
		[[nodiscard]] Mat4& GetLocalMatrix() const;
		[[nodiscard]] Mat4& GetWorldMatrix() const;

		// Hierarchy
		void AddChild(Transform& child);
		void RemoveChild(Transform& child);
		void SetParent(Transform* newParent);

		[[nodiscard]] Transform* GetParent() const { return m_Parent; }
		[[nodiscard]] const std::vector<Transform*>& GetChildren() const { return m_Children; }
		[[nodiscard]] bool IsAncestorOf(const Transform& other) const;
		[[nodiscard]] bool HasChildren() const noexcept { return !m_Children.empty(); }
		[[nodiscard]] bool HasParent() const noexcept { return m_Parent != nullptr; }

		// Mutation helpers
		void Translate(const Vec3& delta);
		void TranslateXZ(const Vec3& delta);
		void Rotate(float pitch = 0.0f, float yaw = 0.0f, float roll = 0.0f);
		void Rotate(const Vec3& euler);
		void LookAt(const Vec3& targetPosition, const Vec3& worldUp = Vec3::Up());
		void LookAt(const Transform& target, const Vec3& worldUp = Vec3::Up());

		void Serialize(Json& out) const final override;
		void Deserialize(const Json& in) final override;

		void OnFieldsChanged() override { MarkDirty(); }
		void SetLocalFromWorld(Vec3 translation, Vec3 rotation, Vec3 scale);

	private:
		void MarkDirty() const;

		[[nodiscard]] bool IsDirty() const;
		void RebuildLocalMatrix() const;
		void RebuildWorldMatrices() const;
		void SetWorldTransform(const Vec3& pos, const Quat& rot, const Vec3& scl);

		static void AssertValidVec3(const Vec3& v, std::string_view msg) {
			if (Any(IsNaN(v))) {
				LW_ERROR(msg);
			}
		}

		static void AssertValidQuat(const Quat& q, std::string_view msg) {
			if (Any(IsNaN(q))) {
				LW_ERROR(msg);
			}
		}

	private:
		Vec3 m_LocalPosition { Vec3::Zero() };
		Vec3 m_LocalScale { Vec3::One() };
	    Quat m_LocalRotation { Quat::Identity() };

	    mutable Mat4 m_LocalMatrix { 1.0f };
	    mutable Mat4 m_WorldMatrix { 1.0f };
	    mutable bool m_Dirty { true };

	    Transform* m_Parent { nullptr };
	    std::vector<Transform*> m_Children{};
	};

	LW_REGISTER_COMPONENT(Transform)

} // namespace Engine::Components

