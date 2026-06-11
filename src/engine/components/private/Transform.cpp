#include "../public/Transform.h"

namespace Engine::Components {

// ─────────────────────────────────────────────
//  Local setters
// ─────────────────────────────────────────────

void Transform::SetLocalPosition(const Vec3& position) {
	m_LocalPosition = position;
	MarkDirty();
}

void Transform::SetLocalScale(const Vec3& scale) {
	CORE_ASSERT(scale.Length() > 0.0001f, "Transform::SetLocalScale: Scale is too small (near zero).");

	m_LocalScale = scale;
	MarkDirty();
}

void Transform::SetLocalRotation(const Quat& q) {
	m_LocalRotation = q;
	MarkDirty();
}

// ─────────────────────────────────────────────
//  World getters
// ─────────────────────────────────────────────

Vec3 Transform::GetWorldPosition() const {
    return GetWorldMatrix().GetTranslation();
}

Quat Transform::GetWorldRotation() const {
    return m_Parent ? m_Parent->GetWorldRotation() * m_LocalRotation : m_LocalRotation;
}

Vec3 Transform::GetWorldScale() const {
    return GetWorldMatrix().GetScale();
}

// ─────────────────────────────────────────────
//  World setters
// ─────────────────────────────────────────────

void Transform::SetWorldPosition(const Vec3& worldPosition) {
	if (m_Parent) {
		const Mat4 parentWorld = m_Parent->GetWorldMatrix();
		const Vec3 parentScale = m_Parent->GetWorldScale();

		// ReSharper disable once CppTooWideScope
		const bool isSingular = Abs(parentScale.x) < 1e-6f || Abs(parentScale.y) < 1e-6f || Abs(parentScale.z) < 1e-6f;

		if (isSingular) {
			m_LocalPosition = worldPosition;
		}
		else {
			const Mat4 invParent = Inverse(parentWorld);
			const Vec4 localPos = invParent * Vec4(worldPosition.x, worldPosition.y, worldPosition.z, 1.0f);

			// 3. Obtener el resultado
			m_LocalPosition = localPos.XYZ();
		}
	}
	else {
		m_LocalPosition = worldPosition;
	}

	MarkDirty();
}

void Transform::SetWorldRotation(const Quat& worldRotation) {
    m_LocalRotation = m_Parent
        ? Inverse(m_Parent->GetWorldRotation()) * worldRotation
        : worldRotation;

    MarkDirty();
}

void Transform::SetWorldScale(const Vec3& worldScale) {
	ASSERT_VALID_VEC3(worldScale, "Transform::SetWorldScale received NaN");
	const Vec3 parentScale = m_Parent ? m_Parent->GetWorldScale() : Vec3(1.0f);

	CORE_ASSERT(parentScale.x != 0.0f && parentScale.y != 0.0f && parentScale.z != 0.0f,
				"Transform::SetWorldScale: Parent has 0 scale, cannot divide.");

	m_LocalScale = worldScale / parentScale;
	MarkDirty();
}

// ─────────────────────────────────────────────
//  Matrices
// ─────────────────────────────────────────────

bool Transform::IsDirty() const {
    return m_Dirty || (m_Parent && m_Parent->IsDirty());
}

void Transform::MarkDirty() const {
    if (!m_Dirty) {
        m_Dirty = true;
        for (const Transform* child : m_Children) {
            child->MarkDirty();
        }
    }
}

void Transform::RebuildLocalMatrix() const {
    m_LocalMatrix = Mat4::Translate(m_LocalPosition)
                  * m_LocalRotation.ToMat4()
                  * Mat4::Scale(m_LocalScale);

    m_Dirty = false;
}

void Transform::RebuildWorldMatrices() const {
    if (m_Dirty) {
        RebuildLocalMatrix();
    }

    m_WorldMatrix = m_Parent ? m_Parent->GetWorldMatrix() * m_LocalMatrix : m_LocalMatrix;
}

const Mat4& Transform::GetLocalMatrix() const {
    if (m_Dirty) {
		RebuildLocalMatrix();
	}

	return m_LocalMatrix;
}

const Mat4& Transform::GetWorldMatrix() const {
    if (IsDirty()) {
		RebuildWorldMatrices();
	}

	return m_WorldMatrix;
}

// ─────────────────────────────────────────────
//  Hierarchy
// ─────────────────────────────────────────────

void Transform::AddChild(Transform* child) {
    if (!child || child == this || child->IsAncestorOf(this)) {
		return;
	}

	if (child->m_Parent) {
        child->m_Parent->RemoveChild(child);
    }

    const Vec3 worldPos = child->GetWorldPosition();
    const Quat worldRot = child->GetWorldRotation();
    const Vec3 worldScl = child->GetWorldScale();

    child->m_Parent = this;
    m_Children.push_back(child);

    child->SetWorldTransform(worldPos, worldRot, worldScl);
}

void Transform::RemoveChild(Transform* child) {
	const auto it = std::ranges::find(m_Children, child);

    if (it == m_Children.end()) {
		return;
	}

	m_Children.erase(it);

    const Vec3 worldPos = child->GetWorldPosition();
    const Quat worldRot = child->GetWorldRotation();
    const Vec3 worldScl = child->GetWorldScale();

    child->m_Parent = nullptr;
    child->SetWorldTransform(worldPos, worldRot, worldScl);
}

void Transform::SetParent(Transform* newParent) {
    if (newParent) {
        newParent->AddChild(this);
    }
	else if (m_Parent) {
        m_Parent->RemoveChild(this);
    }
}

bool Transform::IsAncestorOf(const Transform* other) const {
    const Transform* current = other->m_Parent;

    while (current) {
        if (current == this) return true;
        current = current->m_Parent;
    }

    return false;
}

void Transform::SetWorldTransform(const Vec3& pos, const Quat& rot, const Vec3& scl) {
    SetWorldPosition(pos);
    SetWorldRotation(rot);
    SetWorldScale(scl);
}

// ─────────────────────────────────────────────
//  Mutation helpers
// ─────────────────────────────────────────────

void Transform::Translate(const Vec3& delta) {
    SetLocalPosition(m_LocalPosition + delta);
}

void Transform::TranslateXZ(const Vec3& delta) {
    SetLocalPosition(m_LocalPosition + Vec3(delta.x, 0.0f, delta.z));
}

void Transform::Rotate(const float pitch, const float yaw, const float roll) {
	const auto qPitch = Quat::FromAngleAxis(pitch, Vec3::Right());
	const auto qYaw   = Quat::FromAngleAxis(yaw,   Vec3::Up());
	const auto qRoll  = Quat::FromAngleAxis(roll,  Vec3::Forward());

	SetLocalRotation(m_LocalRotation * qYaw * qPitch * qRoll);
}

void Transform::Rotate(const Vec3& euler) {
    Rotate(euler.x, euler.y, euler.z);
}

void Transform::LookAt(const Transform* target, const Vec3& worldUp) {
    LookAt(target->GetWorldPosition(), worldUp);
}

void Transform::LookAt(const Vec3& targetPosition, const Vec3& worldUp) {
	const Vec3 dirToTarget = targetPosition - GetWorldPosition();
	const float len = dirToTarget.Length();
	CORE_ASSERT(len > 1e-6f, "Transform::LookAt: Target is too close.");

	SetLocalRotation(Quat::LookAt(dirToTarget.Normalized(), worldUp));
}

} // namespace Engine::Components