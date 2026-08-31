
### ENGINE CONVENTION: Left-handed, Y-up, +Z forward
- Vec3::Forward() = (0,0,1), Right() = (1,0,0), Up() = (0,1,0)
- All Transform/Quat math operates in this space
- GLM matrices (TRS, LookRotation) must produce LH results

### RENDER CONVENTION: OpenGL is right-handed, -Z forward in view space
- The ONLY place LH→RH conversion happens is CalculateViewMatrix
- Nowhere else should reference handedness

### TOOLING CONVENTION: ImGuizmo needs an unmirrored camera basis
- ImGuizmo reads the camera basis out of the view matrix and expects its third column to
  point back at the viewer. CalculateViewMatrix folds the LH→RH flip into the view, leaving
  that basis mirrored, and the gizmo then draws the far half of every rotation ring while
  its hit test keeps answering on the near half.
- GizmoViewMatrix / GizmoProjectionMatrix mirror X on both sides of the split, so the two
  cancel in the composed view-projection and only the basis changes.
- The world matrix is never converted. A mirror turns R(axis, angle) into R(axis, -angle),
  so a mirrored world matrix reverses every rotation the gizmo applies in an entity's space.
