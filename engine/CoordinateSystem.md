
### ENGINE CONVENTION: Left-handed, Y-up, +Z forward
- Vec3::Forward() = (0,0,1), Right() = (1,0,0), Up() = (0,1,0)
- All Transform/Quat math operates in this space
- GLM matrices (TRS, LookRotation) must produce LH results

### RENDER CONVENTION: OpenGL is right-handed, -Z forward in view space
- The ONLY place LH→RH conversion happens is CalculateViewMatrix
- Nowhere else should reference handedness
