#include <Leadwort/components/public/Transform.h>
#include <Leadwort/core/math/public/Math.h>
#include <Leadwort/rendering/public/MatrixUtils.h>

#include <gtest/gtest.h>
#include <cmath>

using namespace Leadwort;

TEST(EngineTest, CanaryTest) {
    constexpr int a = 1;
    constexpr int b = 2;

    EXPECT_EQ(a + b, 3);
}

// ─────────────────────────────────────────────
//  Camera / View matrix
// ─────────────────────────────────────────────

TEST(CoordinateSystem, CameraLookingAtCubeAlongPositiveZ) {
    Components::Transform camera;
    camera.SetWorldPosition(Vec3(0, 0, -5));
    camera.LookAt(Vec3(0, 0, 0));

    const Mat4 view = Rendering::MatrixUtils::CalculateViewMatrix(camera);
    const Mat4 proj = Rendering::MatrixUtils::CalculateProjectionMatrix({60.0f, 0.1f, 100.0f, 1.0f});

    const Vec4 clipPos = proj * view * Vec4(0,0,0,1);
    const Vec3 ndc = clipPos.XYZ() / clipPos.w;

    EXPECT_NEAR(ndc.x, 0.0f, 0.01f);
    EXPECT_NEAR(ndc.y, 0.0f, 0.01f);
    EXPECT_GT(ndc.z, -1.0f);
    EXPECT_LT(ndc.z, 1.0f);
}

TEST(CoordinateSystem, MovingCameraForwardMovesTargetFartherInView) {
    Components::Transform camera;
    camera.SetWorldPosition(Vec3(0,0,-5));
    camera.LookAt(Vec3(0,0,0));

    const Mat4 view1 = Rendering::MatrixUtils::CalculateViewMatrix(camera);

    camera.SetWorldPosition(camera.GetWorldPosition() + camera.GetWorldRotation().Forward());
    const Mat4 view2 = Rendering::MatrixUtils::CalculateViewMatrix(camera);

    const Vec4 cubeView1 = view1 * Vec4(0,0,0,1);
    const Vec4 cubeView2 = view2 * Vec4(0,0,0,1);

    EXPECT_GT(cubeView2.z, cubeView1.z);
}

TEST(CoordinateSystem, LookAtFacesTargetForward) {
    Components::Transform camera;
    camera.SetWorldPosition(Vec3(5, 2.5f, -5));
    camera.LookAt(Vec3(0, 0, 0));

    const Vec3 toTarget = Normalize(Vec3(0,0,0) - camera.GetWorldPosition());
    const Vec3 forward  = camera.GetForward();

    EXPECT_NEAR(forward.x, toTarget.x, 0.01f);
    EXPECT_NEAR(forward.y, toTarget.y, 0.01f);
    EXPECT_NEAR(forward.z, toTarget.z, 0.01f);
}

TEST(CoordinateSystem, LookAtKeepsUpRoughlyAligned) {
    Components::Transform camera;
    camera.SetWorldPosition(Vec3(5, 2.5f, -5));
    camera.LookAt(Vec3(0, 0, 0));

    EXPECT_GT(camera.GetUp().y, 0.0f);
}

TEST(CoordinateSystem, ForwardRightUpAreOrthonormal) {
    Components::Transform t;
    t.SetWorldRotation(Quat::FromEuler(35.0f, 120.0f, -15.0f));

    const Vec3 f = t.GetForward();
    const Vec3 r = t.GetRight();
    const Vec3 u = t.GetUp();

    EXPECT_NEAR(f.Length(), 1.0f, 1e-4f);
    EXPECT_NEAR(r.Length(), 1.0f, 1e-4f);
    EXPECT_NEAR(u.Length(), 1.0f, 1e-4f);

    EXPECT_NEAR(Dot(f, r), 0.0f, 1e-4f);
    EXPECT_NEAR(Dot(f, u), 0.0f, 1e-4f);
    EXPECT_NEAR(Dot(r, u), 0.0f, 1e-4f);
}

TEST(CoordinateSystem, IdentityTransformLooksAlongConventionForward) {
    const Components::Transform t;

    EXPECT_NEAR(t.GetForward().x, Vec3::Forward().x, 1e-5f);
    EXPECT_NEAR(t.GetForward().y, Vec3::Forward().y, 1e-5f);
    EXPECT_NEAR(t.GetForward().z, Vec3::Forward().z, 1e-5f);

    EXPECT_NEAR(t.GetRight().x, Vec3::Right().x, 1e-5f);
    EXPECT_NEAR(t.GetUp().y,    Vec3::Up().y,    1e-5f);
}

// NUEVO: Verifica que el cambio de Aspect Ratio modifique correctamente NDC para evitar "stretch"
TEST(CoordinateSystem, ProjectionAdjustsToAspectRatio) {
    const Mat4 projSquare = Rendering::MatrixUtils::CalculateProjectionMatrix({60.0f, 0.1f, 100.0f, 1.0f});
    const Mat4 projWide   = Rendering::MatrixUtils::CalculateProjectionMatrix({60.0f, 0.1f, 100.0f, 2.0f}); // Doble de ancho

	constexpr Vec4 viewPos(1.0f, 0.0f, -5.0f, 1.0f); // Un punto desplazado a la derecha en View Space

    const Vec4 clipSquare = projSquare * viewPos;
    const Vec4 clipWide   = projWide * viewPos;

    const float ndcXSquare = clipSquare.x / clipSquare.w;
    const float ndcXWide   = clipWide.x / clipWide.w;

    // Al ser el viewport el doble de ancho, el mismo punto 3D debe proyectarse a la mitad de distancia del centro en NDC
    EXPECT_NEAR(ndcXSquare, ndcXWide * 2.0f, 0.01f);
}

// NUEVO: Verifica que los límites del Frustum (Near y Far) clipen correctamente en OpenGL (-1 a 1)
TEST(CoordinateSystem, ProjectionClipsOutsideNearAndFarPlanes) {
    const Mat4 proj = Rendering::MatrixUtils::CalculateProjectionMatrix({60.0f, 0.1f, 100.0f, 1.0f});

    // Punto exactamente en el plano Near (Z = -0.1 en View Space de OpenGL)
    const Vec4 clipNear = proj * Vec4(0.0f, 0.0f, -0.1f, 1.0f);
    EXPECT_NEAR(clipNear.z / clipNear.w, -1.0f, 0.01f);

    // Punto exactamente en el plano Far (Z = -100.0 en View Space de OpenGL)
    const Vec4 clipFar = proj * Vec4(0.0f, 0.0f, -100.0f, 1.0f);
    EXPECT_NEAR(clipFar.z / clipFar.w, 1.0f, 0.01f);

    // Punto demasiado cercano (detrás del Near plane) -> Debe salirse del rango NDC [-1, 1]
    const Vec4 clipTooClose = proj * Vec4(0.0f, 0.0f, -0.05f, 1.0f);
    EXPECT_LT(clipTooClose.z / clipTooClose.w, -1.0f);
}

// ─────────────────────────────────────────────
//  LookRotation edge cases
// ─────────────────────────────────────────────

TEST(CoordinateSystem, LookRotationZeroForwardReturnsIdentity) {
    const Quat q = Quat::LookRotation(Vec3::Zero(), Vec3::Up());

    EXPECT_EQ(q.x, Quat::Identity().x);
    EXPECT_EQ(q.y, Quat::Identity().y);
    EXPECT_EQ(q.z, Quat::Identity().z);
    EXPECT_EQ(q.w, Quat::Identity().w);
}

TEST(CoordinateSystem, LookRotationParallelToUpDoesNotProduceNaN) {
    const Quat q = Quat::LookRotation(Vec3::Up(), Vec3::Up());

    EXPECT_FALSE(std::isnan(q.x));
    EXPECT_FALSE(std::isnan(q.y));
    EXPECT_FALSE(std::isnan(q.z));
    EXPECT_FALSE(std::isnan(q.w));

    const Vec3 f = (q * Vec3::Forward());
    EXPECT_NEAR(f.x, Vec3::Up().x, 0.01f);
    EXPECT_NEAR(f.y, Vec3::Up().y, 0.01f);
    EXPECT_NEAR(f.z, Vec3::Up().z, 0.01f);
}

TEST(CoordinateSystem, LookRotationProducesUnitQuaternion) {
    const Quat q = Quat::LookRotation(Vec3(1, 1, 1), Vec3::Up());
    const float lenSq = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w;

    EXPECT_NEAR(lenSq, 1.0f, 1e-4f);
}

// NUEVO: Evita la singularidad (Gimbal lock/NaNs) cuando la cámara mira directamente hacia abajo en el eje Y
TEST(CoordinateSystem, LookAtStraightDownHandlesUpVectorGracefully) {
    Components::Transform camera;
    camera.SetWorldPosition(Vec3(0.0f, 10.0f, 0.0f));
    camera.LookAt(Vec3(0.0f, 0.0f, 0.0f)); // Mirando verticalmente hacia abajo

    EXPECT_NEAR(camera.GetForward().y, -1.0f, 0.01f);

    // Validar que los vectores resultantes no colapsen en NaN o longitudes cero
    EXPECT_FALSE(std::isnan(camera.GetRight().x));
    EXPECT_FALSE(std::isnan(camera.GetUp().z));
    EXPECT_NEAR(camera.GetRight().Length(), 1.0f, 1e-4f);
}

// ─────────────────────────────────────────────
//  Mat4 basics
// ─────────────────────────────────────────────

TEST(Mat4Math, IdentityTimesVectorIsUnchanged) {
    const Mat4 id = Mat4::Identity();
	constexpr Vec4 v(1, 2, 3, 1);

    const Vec4 result = id * v;

    EXPECT_NEAR(result.x, 1.0f, 1e-5f);
    EXPECT_NEAR(result.y, 2.0f, 1e-5f);
    EXPECT_NEAR(result.z, 3.0f, 1e-5f);
    EXPECT_NEAR(result.w, 1.0f, 1e-5f);
}

TEST(Mat4Math, InverseOfTranslationNegatesTranslation) {
    const Mat4 t = Mat4::Translate(Vec3(5, -3, 2));
    const Mat4 inv = t.Inversed();

    const Vec3 translation = inv.GetTranslation();
    EXPECT_NEAR(translation.x, -5.0f, 1e-5f);
    EXPECT_NEAR(translation.y, 3.0f, 1e-5f);
    EXPECT_NEAR(translation.z, -2.0f, 1e-5f);
}

TEST(Mat4Math, MatrixTimesItsInverseIsIdentity) {
    const Mat4 m = Mat4::TRS(
        Vec3(3, -1, 7),
        Quat::FromEuler(20.0f, 40.0f, -10.0f),
        Vec3(1.5f, 1.0f, 0.5f)
    );

    const Mat4 result = m * m.Inversed();
    const Mat4 identity = Mat4::Identity();

    for (int col = 0; col < 4; ++col) {
        const Vec4 a = result.GetColumn(col);
        const Vec4 b = identity.GetColumn(col);

        EXPECT_NEAR(a.x, b.x, 1e-4f);
        EXPECT_NEAR(a.y, b.y, 1e-4f);
        EXPECT_NEAR(a.z, b.z, 1e-4f);
        EXPECT_NEAR(a.w, b.w, 1e-4f);
    }
}

TEST(Mat4Math, TRSDecomposeRoundTrip) {
    constexpr Vec3 pos(2, 3, -4);
    constexpr Vec3 scale(1, 2, 0.5f);
    const Quat rot = Quat::FromAngleAxis(33.0f, Normalize(Vec3(1,1,0)));

    const Mat4 m = Mat4::TRS(pos, rot, scale);

    const Vec3 outPos   = m.GetTranslation();
    const Vec3 outScale = m.GetScale();

    EXPECT_NEAR(outPos.x, pos.x, 1e-4f);
    EXPECT_NEAR(outPos.y, pos.y, 1e-4f);
    EXPECT_NEAR(outPos.z, pos.z, 1e-4f);

    EXPECT_NEAR(outScale.x, scale.x, 1e-4f);
    EXPECT_NEAR(outScale.y, scale.y, 1e-4f);
    EXPECT_NEAR(outScale.z, scale.z, 1e-4f);
}

// ─────────────────────────────────────────────
//  Quat basics
// ─────────────────────────────────────────────

TEST(QuatMath, IdentityRotationDoesNotChangeVector) {
    constexpr Vec3 v(1, 2, 3);
    const Vec3 rotated = Quat::Identity() * v;

    EXPECT_NEAR(rotated.x, v.x, 1e-5f);
    EXPECT_NEAR(rotated.y, v.y, 1e-5f);
    EXPECT_NEAR(rotated.z, v.z, 1e-5f);
}

TEST(QuatMath, Rotate90DegreesAroundUpMapsForwardToRight) {
    const Quat q = Quat::FromAngleAxis(90.0f, Vec3::Up());
    const Vec3 rotated = q * Vec3::Forward();

    const float dotRight = Dot(rotated, Vec3::Right());
    EXPECT_GT(std::abs(dotRight), 0.9f);
}

TEST(QuatMath, ConjugateUndoesRotation) {
    const Quat q = Quat::FromAngleAxis(57.0f, Normalize(Vec3(1,1,1)));
    constexpr Vec3 v(1, 0, 0);

    const Vec3 rotated   = q * v;
    const Vec3 unrotated = q.Conjugate() * rotated;

    EXPECT_NEAR(unrotated.x, v.x, 1e-4f);
    EXPECT_NEAR(unrotated.y, v.y, 1e-4f);
    EXPECT_NEAR(unrotated.z, v.z, 1e-4f);
}

TEST(QuatMath, QuatMultiplicationIsAssociative) {
    const Quat a = Quat::FromAngleAxis(20.0f, Vec3::Up());
    const Quat b = Quat::FromAngleAxis(40.0f, Vec3::Right());
    const Quat c = Quat::FromAngleAxis(15.0f, Vec3::Forward());

    const Quat lhs = (a * b) * c;
    const Quat rhs = a * (b * c);

    EXPECT_NEAR(lhs.x, rhs.x, 1e-4f);
    EXPECT_NEAR(lhs.y, rhs.y, 1e-4f);
    EXPECT_NEAR(lhs.z, rhs.z, 1e-4f);
    EXPECT_NEAR(lhs.w, rhs.w, 1e-4f);
}

TEST(QuatMath, SlerpAtZeroIsStart) {
    constexpr Quat a = Quat::Identity();
    const Quat b = Quat::FromAngleAxis(90.0f, Vec3::Up());

    const Quat result = a.Slerp(b, 0.0f);

    EXPECT_NEAR(result.x, a.x, 1e-4f);
    EXPECT_NEAR(result.y, a.y, 1e-4f);
    EXPECT_NEAR(result.z, a.z, 1e-4f);
    EXPECT_NEAR(result.w, a.w, 1e-4f);
}

TEST(QuatMath, SlerpAtOneIsEnd) {
    constexpr Quat a = Quat::Identity();
    const Quat b = Quat::FromAngleAxis(90.0f, Vec3::Up());

    const Quat result = a.Slerp(b, 1.0f);

    EXPECT_NEAR(result.x, b.x, 1e-4f);
    EXPECT_NEAR(result.y, b.y, 1e-4f);
    EXPECT_NEAR(result.z, b.z, 1e-4f);
    EXPECT_NEAR(result.w, b.w, 1e-4f);
}

// NUEVO: Asegura que el punto medio de un Slerp represente exactamente la mitad de la rotación geométrica
TEST(QuatMath, SlerpAtMidpointIsHalfway) {
	constexpr Quat a = Quat::Identity();
    const Quat b = Quat::FromAngleAxis(90.0f, Vec3::Up());

    const Quat midpoint = a.Slerp(b, 0.5f);
    const Vec3 rotated = midpoint * Vec3::Forward();

    // A 45 grados en el eje Y, los componentes X y Z del vector Forward resultante deben ser idénticos
    EXPECT_NEAR(rotated.x, rotated.z, 1e-4f);
    EXPECT_GT(rotated.x, 0.0f);
}

//  FirstPersonController-style incremental rotation

TEST(CoordinateSystem, IncrementalYawPreservesForwardLength) {
    Quat rotation = Quat::Identity();

    for (int i = 0; i < 50; ++i) {
        const Quat qYaw = Quat::FromAngleAxis(2.0f, Vec3::Up());
        rotation = (qYaw * rotation).Normalized();
    }

    const Vec3 forward = rotation * Vec3::Forward();
    EXPECT_NEAR(forward.Length(), 1.0f, 1e-3f);
}

TEST(CoordinateSystem, FullYawRotationReturnsToStart) {
    Quat rotation = Quat::Identity();

    for (int i = 0; i < 180; i++) {
        const Quat qYaw = Quat::FromAngleAxis(2.0f, Vec3::Up());
        rotation = (qYaw * rotation).Normalized();
    }

    const Vec3 forward = rotation * Vec3::Forward();

    EXPECT_NEAR(forward.x, Vec3::Forward().x, 1e-2f);
    EXPECT_NEAR(forward.y, Vec3::Forward().y, 1e-2f);
    EXPECT_NEAR(forward.z, Vec3::Forward().z, 1e-2f);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}