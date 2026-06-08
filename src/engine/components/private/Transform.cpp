#include "../public/Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <algorithm>
#include <cmath>

namespace Engine::Components {

// ─────────────────────────────────────────────
//  Local setters
// ─────────────────────────────────────────────

void Transform::SetLocalPosition(const glm::vec3& position) {
	ASSERT_VALID_VEC3(position, "Transform::SetLocalPosition received NaN");
	m_LocalPosition = position;
	MarkDirty();
}

void Transform::SetLocalScale(const glm::vec3& scale) {
	ASSERT_VALID_VEC3(scale, "Transform::SetLocalScale received NaN");
	CORE_ASSERT(glm::length(scale) > 0.0001f, "Transform::SetLocalScale: Scale is too small (near zero).");

	m_LocalScale = scale;
	MarkDirty();
}

void Transform::SetLocalRotation(const glm::quat& q) {
	ASSERT_VALID_QUAT(q, "Transform::SetLocalRotation received NaN");
	m_LocalRotation = q;
	MarkDirty();
}

// ─────────────────────────────────────────────
//  World getters
// ─────────────────────────────────────────────

glm::vec3 Transform::GetWorldPosition() const {
    return GetWorldMatrix()[3];
}

glm::quat Transform::GetWorldRotation() const {
    return m_Parent ? m_Parent->GetWorldRotation() * m_LocalRotation : m_LocalRotation;
}

glm::vec3 Transform::GetWorldScale() const {
    const glm::mat4& wm = GetWorldMatrix();

    return {
        glm::length(glm::vec3(wm[0])),
        glm::length(glm::vec3(wm[1])),
        glm::length(glm::vec3(wm[2]))
    };
}

// ─────────────────────────────────────────────
//  World setters
// ─────────────────────────────────────────────

void Transform::SetWorldPosition(const glm::vec3& worldPosition) {
	ASSERT_VALID_VEC3(worldPosition, "Transform::SetWorldPosition received NaN");

	if (m_Parent) {
		const glm::mat4 parentWorld = m_Parent->GetWorldMatrix();
		const glm::vec3 parentScale = m_Parent->GetWorldScale();

		const bool isSingular = glm::abs(parentScale.x) < 1e-6f || glm::abs(parentScale.y) < 1e-6f || glm::abs(parentScale.z) < 1e-6f;

		if (isSingular) {
			m_LocalPosition = worldPosition;
		}
		else {
			const glm::mat4 invParent = glm::inverse(parentWorld);
			m_LocalPosition = glm::vec3(invParent * glm::vec4(worldPosition, 1.0f));
		}
	}
	else {
		m_LocalPosition = worldPosition;
	}

	MarkDirty();
}

void Transform::SetWorldRotation(const glm::quat& worldRotation) {
    m_LocalRotation = m_Parent
        ? glm::inverse(m_Parent->GetWorldRotation()) * worldRotation
        : worldRotation;

    MarkDirty();
}

void Transform::SetWorldScale(const glm::vec3& worldScale) {
	ASSERT_VALID_VEC3(worldScale, "Transform::SetWorldScale received NaN");
	const glm::vec3 parentScale = m_Parent ? m_Parent->GetWorldScale() : glm::vec3(1.0f);

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
    m_LocalMatrix = glm::translate(glm::mat4(1.0f), m_LocalPosition)
                  * glm::toMat4(m_LocalRotation)
                  * glm::scale(glm::mat4(1.0f), m_LocalScale);

    m_Dirty = false;
}

void Transform::RebuildWorldMatrices() const {
    if (m_Dirty) {
        RebuildLocalMatrix();
    }

    m_WorldMatrix = m_Parent ? m_Parent->GetWorldMatrix() * m_LocalMatrix : m_LocalMatrix;
}

const glm::mat4& Transform::GetLocalMatrix() const {
    if (m_Dirty) {
		RebuildLocalMatrix();
	}

	return m_LocalMatrix;
}

const glm::mat4& Transform::GetWorldMatrix() const {
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

    const glm::vec3 worldPos = child->GetWorldPosition();
    const glm::quat worldRot = child->GetWorldRotation();
    const glm::vec3 worldScl = child->GetWorldScale();

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

    const glm::vec3 worldPos = child->GetWorldPosition();
    const glm::quat worldRot = child->GetWorldRotation();
    const glm::vec3 worldScl = child->GetWorldScale();

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

void Transform::SetWorldTransform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scl) {
    SetWorldPosition(pos);
    SetWorldRotation(rot);
    SetWorldScale(scl);
}

// ─────────────────────────────────────────────
//  Mutation helpers
// ─────────────────────────────────────────────

void Transform::Translate(const glm::vec3& delta) {
    SetLocalPosition(m_LocalPosition + delta);
}

void Transform::TranslateXZ(const glm::vec3& delta) {
    SetLocalPosition(m_LocalPosition + glm::vec3(delta.x, 0.0f, delta.z));
}

void Transform::Rotate(const float pitch, const float yaw, const float roll) {
	const auto qPitch = glm::angleAxis(pitch, glm::vec3(1, 0, 0));
	const auto qYaw   = glm::angleAxis(yaw,   glm::vec3(0, 1, 0));
	const auto qRoll  = glm::angleAxis(roll,  glm::vec3(0, 0, 1));

	SetLocalRotation(m_LocalRotation * qYaw * qPitch * qRoll);
}

void Transform::Rotate(const glm::vec3& euler) {
    Rotate(euler.x, euler.y, euler.z);
}

void Transform::LookAt(const Transform* target, const glm::vec3& worldUp) {
    LookAt(target->GetWorldPosition(), worldUp);
}

void Transform::LookAt(const glm::vec3& targetPosition, const glm::vec3& worldUp) {
	const glm::vec3 dirToTarget = targetPosition - GetWorldPosition();

	const float len = glm::length(dirToTarget);
	CORE_ASSERT(len > 1e-6f, "Transform::LookAt: Target is too close to current position (cannot normalize).");

	const glm::vec3 forward = glm::normalize(dirToTarget);
	SetLocalRotation(glm::quatLookAtLH(forward, worldUp));
}

} // namespace Engine::Components