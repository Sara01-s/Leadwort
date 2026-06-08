#pragma once
#include "engine/utils/public/Logger.h"
#include "engine/components/Component.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/component_wise.hpp>
#include <vector>

namespace Engine::Components {

#define ASSERT_VALID_VEC3(v, msg) CORE_ASSERT(!glm::any(glm::isnan(v)), msg)
#define ASSERT_VALID_QUAT(q, msg) CORE_ASSERT(!glm::any(glm::isnan(q)), msg)

class Transform : public Component {
public:

    // ── Local space ──────────────────────────────────────────────────────────

    void SetLocalPosition(const glm::vec3& position);
    void SetLocalScale(const glm::vec3& scale);
    void SetLocalRotation(const glm::quat& rotation);

    [[nodiscard]] const glm::vec3& GetLocalPosition() const { return m_LocalPosition; }
    [[nodiscard]] const glm::vec3& GetLocalScale()    const { return m_LocalScale; }
    [[nodiscard]] const glm::quat& GetLocalRotation() const { return m_LocalRotation; }

    [[nodiscard]] glm::vec3 GetForward() const { return GetWorldRotation() * glm::vec3(0, 0, 1); }
    [[nodiscard]] glm::vec3 GetRight()   const { return GetWorldRotation() * glm::vec3(1, 0, 0); }
    [[nodiscard]] glm::vec3 GetUp()      const { return GetWorldRotation() * glm::vec3(0, 1, 0); }

    // ── World space ──────────────────────────────────────────────────────────

    [[nodiscard]] glm::vec3 GetWorldPosition() const;
    [[nodiscard]] glm::quat GetWorldRotation() const;
    [[nodiscard]] glm::vec3 GetWorldScale()    const;

    void SetWorldPosition(const glm::vec3& worldPosition);
    void SetWorldRotation(const glm::quat& worldRotation);
    void SetWorldScale(const glm::vec3& worldScale);

    // ── Matrices ─────────────────────────────────────────────────────────────

    [[nodiscard]] const glm::mat4& GetLocalMatrix() const;
    [[nodiscard]] const glm::mat4& GetWorldMatrix() const;

    // ── Hierarchy ────────────────────────────────────────────────────────────

    void AddChild(Transform* child);
    void RemoveChild(Transform* child);
    void SetParent(Transform* newParent);

    [[nodiscard]] Transform* GetParent() const { return m_Parent; }
    [[nodiscard]] const std::vector<Transform*>& GetChildren() const { return m_Children; }
    [[nodiscard]] bool IsAncestorOf(const Transform* other) const;

    // ── Mutation helpers ─────────────────────────────────────────────────────

    void Translate(const glm::vec3& delta);
    void TranslateXZ(const glm::vec3& delta);
    void Rotate(float pitch = 0.0f, float yaw = 0.0f, float roll = 0.0f);
    void Rotate(const glm::vec3& euler);
    void LookAt(const Transform* target, const glm::vec3& worldUp = {0, 1, 0});
    void LookAt(const glm::vec3& targetPosition, const glm::vec3& worldUp = {0, 1, 0});

private:
	[[nodiscard]] bool IsDirty() const;
	void MarkDirty() const;
	void RebuildLocalMatrix() const;
	void RebuildWorldMatrices() const;
	void SetWorldTransform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scl);

    glm::vec3 m_LocalPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_LocalScale = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::quat m_LocalRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    mutable glm::mat4 m_LocalMatrix{1.0f};
    mutable glm::mat4 m_WorldMatrix{1.0f};
    mutable bool m_Dirty = true;

    Transform* m_Parent = nullptr;
    std::vector<Transform*> m_Children;
};

} // namespace Engine::Components