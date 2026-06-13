#include <gtest/gtest.h>
#include "engine/components/public/Transform.h"

using namespace Engine;

TEST(TransformHierarchy, ChildWorldPositionAccountsForParent) {
    Components::Transform parent;
    Components::Transform child;

    parent.SetWorldPosition(Vec3(10, 0, 0));
    parent.AddChild(child);

    child.SetLocalPosition(Vec3(0, 5, 0));

    const Vec3 worldPos = child.GetWorldPosition();
    EXPECT_NEAR(worldPos.x, 10.0f, 1e-4f);
    EXPECT_NEAR(worldPos.y, 5.0f, 1e-4f);
    EXPECT_NEAR(worldPos.z, 0.0f, 1e-4f);
}

TEST(TransformHierarchy, AddChildPreservesWorldPosition) {
    Components::Transform parent;
    Components::Transform child;

    parent.SetWorldPosition(Vec3(10, 0, 0));
    child.SetWorldPosition(Vec3(15, 5, 0));

    parent.AddChild(child);

    const Vec3 worldPos = child.GetWorldPosition();
    EXPECT_NEAR(worldPos.x, 15.0f, 1e-4f);
    EXPECT_NEAR(worldPos.y, 5.0f, 1e-4f);
    EXPECT_NEAR(worldPos.z, 0.0f, 1e-4f);
}

TEST(TransformHierarchy, RemoveChildPreservesWorldPosition) {
    Components::Transform parent;
    Components::Transform child;

    parent.SetWorldPosition(Vec3(10, 0, 0));
    parent.AddChild(child);
    child.SetLocalPosition(Vec3(5, 0, 0));

    const Vec3 worldBefore = child.GetWorldPosition();
    parent.RemoveChild(&child);
    const Vec3 worldAfter = child.GetWorldPosition();

    EXPECT_NEAR(worldBefore.x, worldAfter.x, 1e-4f);
    EXPECT_NEAR(worldBefore.y, worldAfter.y, 1e-4f);
    EXPECT_NEAR(worldBefore.z, worldAfter.z, 1e-4f);
}


TEST(TransformHierarchy, SetParentNullDetachesFromParent) {
    Components::Transform parent;
    Components::Transform child;

    parent.AddChild(child);
    EXPECT_EQ(child.GetParent(), &parent);

    child.SetParent(nullptr);
    EXPECT_EQ(child.GetParent(), nullptr);
    EXPECT_TRUE(parent.GetChildren().empty());
}

TEST(Transform, SetWorldPositionWithoutParentSetsLocalDirectly) {
	Components::Transform t;
	t.SetWorldPosition(Vec3(1, 2, 3));

	EXPECT_NEAR(t.GetLocalPosition().x, 1.0f, 1e-5f);
	EXPECT_NEAR(t.GetLocalPosition().y, 2.0f, 1e-5f);
	EXPECT_NEAR(t.GetLocalPosition().z, 3.0f, 1e-5f);
}

TEST(Transform, SetWorldRotationWithoutParentSetsLocalDirectly) {
	Components::Transform t;
	const Quat rot = Quat::FromAngleAxis(45.0f, Vec3::Up());
	t.SetWorldRotation(rot);

	EXPECT_NEAR(t.GetLocalRotation().x, rot.x, 1e-5f);
	EXPECT_NEAR(t.GetLocalRotation().y, rot.y, 1e-5f);
	EXPECT_NEAR(t.GetLocalRotation().z, rot.z, 1e-5f);
	EXPECT_NEAR(t.GetLocalRotation().w, rot.w, 1e-5f);
}

TEST(Transform, TranslateMovesAlongLocalAxes) {
	Components::Transform t;
	t.SetLocalPosition(Vec3::Zero());

	t.Translate(Vec3(1, 2, 3));

	EXPECT_NEAR(t.GetLocalPosition().x, 1.0f, 1e-5f);
	EXPECT_NEAR(t.GetLocalPosition().y, 2.0f, 1e-5f);
	EXPECT_NEAR(t.GetLocalPosition().z, 3.0f, 1e-5f);
}

TEST(Transform, TranslateXZIgnoresYComponent) {
	Components::Transform t;
	t.SetLocalPosition(Vec3(0, 5, 0));

	t.TranslateXZ(Vec3(1, 99, 1));

	EXPECT_NEAR(t.GetLocalPosition().x, 1.0f, 1e-5f);
	EXPECT_NEAR(t.GetLocalPosition().y, 5.0f, 1e-5f);
	EXPECT_NEAR(t.GetLocalPosition().z, 1.0f, 1e-5f);
}

TEST(Transform, WorldMatrixIsIdentityByDefault) {
	Components::Transform t;
	const Mat4& world = t.GetWorldMatrix();

	EXPECT_EQ(world, Mat4::Identity());
}

TEST(Transform, GetWorldMatrixCachesUntilDirty) {
	Components::Transform t;

	const Mat4 first = t.GetWorldMatrix();
	const Mat4 second = t.GetWorldMatrix();

	EXPECT_EQ(first, second);

	t.SetLocalPosition(Vec3(1, 0, 0));
	const Mat4 third = t.GetWorldMatrix();

	EXPECT_NE(first, third);
}

TEST(TransformHierarchy, ParentRotationAffectsChildWorldPosition) {
    Components::Transform parent;
    Components::Transform child;

    parent.AddChild(child);
    child.SetLocalPosition(Vec3(1, 0, 0));

    // Rotar 90 grados en Y debería mover el hijo de +X a +/-Z
    parent.SetWorldRotation(Quat::FromAngleAxis(90.0f, Vec3::Up()));

    const Vec3 worldPos = child.GetWorldPosition();
    EXPECT_NEAR(worldPos.x, 0.0f, 1e-3f);
    EXPECT_NEAR(worldPos.y, 0.0f, 1e-3f);
    EXPECT_GT(std::abs(worldPos.z), 0.9f);
}

TEST(TransformHierarchy, MarkDirtyPropagatesToChildren) {
    Components::Transform parent;
    Components::Transform child;
    parent.AddChild(child);

    // Forzar el cálculo inicial
    (void)child.GetWorldMatrix();

    parent.SetLocalPosition(Vec3(1, 2, 3));

    const Vec3 worldPos = child.GetWorldPosition();
    EXPECT_NEAR(worldPos.x, 1.0f, 1e-4f);
    EXPECT_NEAR(worldPos.y, 2.0f, 1e-4f);
    EXPECT_NEAR(worldPos.z, 3.0f, 1e-4f);
}

TEST(TransformHierarchy, CannotAddSelfAsChild) {
    Components::Transform t;
    t.AddChild(t);

    EXPECT_TRUE(t.GetChildren().empty());
}

TEST(TransformHierarchy, CannotAddAncestorAsChild) {
    Components::Transform grandparent;
    Components::Transform parent;
    Components::Transform child;

    grandparent.AddChild(parent);
    parent.AddChild(child);
    child.AddChild(grandparent);

	EXPECT_TRUE(grandparent.GetParent() == nullptr);
	EXPECT_FALSE(child.IsAncestorOf(&grandparent));
	EXPECT_TRUE(grandparent.IsAncestorOf(&child));
}
