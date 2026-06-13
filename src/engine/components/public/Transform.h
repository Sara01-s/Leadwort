#pragma once
#include "engine/components/Component.h"
#include "engine/core/math/public/Math.h"
#include "engine/core/math/public/Mat4.h"
#include "engine/core/math/public/Quat.h"
#include "engine/core/math/public/Vec3.h"
#include "engine/utils/public/Logger.h"

#include <vector>

namespace Engine::Components {

#define ASSERT_VALID_VEC3(v, msg) if (Any(IsNaN(v))) { CORE_ERROR(msg); }
#define ASSERT_VALID_QUAT(q, msg) if (Any(IsNaN(q))) { CORE_ERROR(msg); }

class Transform : public Component {
public:

	// ── Local space ──────────────────────────────────────────────────────────

	void SetLocalPosition(const Vec3& position);
	void SetLocalScale(const Vec3& scale);
	void SetLocalRotation(const Quat& rotation);

	[[nodiscard]] const Vec3& GetLocalPosition() const { return m_LocalPosition; }
	[[nodiscard]] const Vec3& GetLocalScale()    const { return m_LocalScale; }
	[[nodiscard]] const Quat& GetLocalRotation() const { return m_LocalRotation; }

	[[nodiscard]] Vec3 GetForward() const { return GetWorldRotation() * Vec3::Forward(); }
	[[nodiscard]] Vec3 GetRight()   const { return GetWorldRotation() * Vec3::Right(); }
	[[nodiscard]] Vec3 GetUp()      const { return GetWorldRotation() * Vec3::Up(); }

	// ── World space ──────────────────────────────────────────────────────────

	[[nodiscard]] Vec3 GetWorldPosition() const;
	[[nodiscard]] Quat GetWorldRotation() const;
	[[nodiscard]] Vec3 GetWorldScale()    const;

	void SetWorldPosition(const Vec3& worldPosition);
	void SetWorldRotation(const Quat& worldRotation);
	void SetWorldScale(const Vec3& worldScale);

	// ── Matrices ─────────────────────────────────────────────────────────────

	[[nodiscard]] const Mat4& GetLocalMatrix() const;
	[[nodiscard]] const Mat4& GetWorldMatrix() const;

	// ── Hierarchy ────────────────────────────────────────────────────────────

	void AddChild(Transform& child);
	void RemoveChild(Transform& child);
	void SetParent(Transform* newParent);

	[[nodiscard]] Transform* GetParent() const { return m_Parent; }
	[[nodiscard]] const std::vector<Transform*>& GetChildren() const { return m_Children; }
	[[nodiscard]] bool IsAncestorOf(const Transform& other) const;

	// ── Mutation helpers ─────────────────────────────────────────────────────

	void Translate(const Vec3& delta);
	void TranslateXZ(const Vec3& delta);
	void Rotate(float pitch = 0.0f, float yaw = 0.0f, float roll = 0.0f);
	void Rotate(const Vec3& euler);
	void LookAt(const Vec3& targetPosition, const Vec3& worldUp = Vec3::Up());
	void LookAt(const Transform& target, const Vec3& worldUp = Vec3::Up());

private:
	[[nodiscard]] bool IsDirty() const;
	void MarkDirty() const;
	void RebuildLocalMatrix() const;
	void RebuildWorldMatrices() const;
	void SetWorldTransform(const Vec3& pos, const Quat& rot, const Vec3& scl);

	Vec3 m_LocalPosition { Vec3::Zero() };
	Vec3 m_LocalScale { Vec3::One() };
    Quat m_LocalRotation { Quat::Identity() };

    mutable Mat4 m_LocalMatrix { 1.0f };
    mutable Mat4 m_WorldMatrix { 1.0f };
    mutable bool m_Dirty { true };

    Transform* m_Parent { nullptr };
    std::vector<Transform*> m_Children{};
};

} // namespace Engine::Components