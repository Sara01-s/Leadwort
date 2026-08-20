#pragma once

#include "CubeMapTexture.h"
#include <Leadwort/core/public/Core.h>

#include <glad/glad.h>

namespace Leadwort::Rendering::Bindables { class Texture; }

namespace Leadwort::Rendering {

    class IBLBaker {
    public:
        struct Result {
            Unique<CubeMapTexture> EnvironmentCubemap;
            Unique<CubeMapTexture> IrradianceMap;
            Unique<CubeMapTexture> PrefilterMap;
            GLuint BrdfLUT { 0 };

            Result() = default;
            ~Result() {
                if (BrdfLUT != 0) {
                    glDeleteTextures(1, &BrdfLUT);
                }
            }

            Result(const Result&) = delete;
            Result& operator=(const Result&) = delete;

            Result(Result&& other) noexcept
                : EnvironmentCubemap(std::move(other.EnvironmentCubemap))
                , IrradianceMap(std::move(other.IrradianceMap))
                , PrefilterMap(std::move(other.PrefilterMap))
                , BrdfLUT(other.BrdfLUT)
            {
                other.BrdfLUT = 0;
            }

            Result& operator=(Result&& other) noexcept {
                if (this != &other) {
                    if (BrdfLUT != 0) {
                        glDeleteTextures(1, &BrdfLUT);
                    }

                    EnvironmentCubemap = std::move(other.EnvironmentCubemap);
                    IrradianceMap      = std::move(other.IrradianceMap);
                    PrefilterMap       = std::move(other.PrefilterMap);
                    BrdfLUT            = other.BrdfLUT;
                    other.BrdfLUT      = 0;
                }
                return *this;
            }
        };

        static Result Bake(const Bindables::Texture& equirectangularHDRI);

    private:
        struct ScratchFBO {
            GLuint fbo { 0 };
            GLuint rbo { 0 };

            ScratchFBO(int width, int height);
            ~ScratchFBO();

            ScratchFBO(const ScratchFBO&) = delete;
            ScratchFBO& operator=(const ScratchFBO&) = delete;

            void AttachCubeFace(GLuint cubemapID, int face, int mip = 0) const;
        };

        static Unique<CubeMapTexture> EquirectToCubemap(const Bindables::Texture& hdri, int resolution);
        static Unique<CubeMapTexture> ConvolveIrradiance(const CubeMapTexture& envCubemap, int resolution);
        static Unique<CubeMapTexture> PrefilterEnvironment(const CubeMapTexture& envCubemap, int resolution, int mipLevels);
        static GLuint GenerateBrdfLUT(int resolution);

        static void RenderCube();
        static void RenderFullscreenQuad();
    };

}