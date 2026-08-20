#include "Leadwort/rendering/IBLBaker.h"

#include <Leadwort/asset-management/public/AssetDatabase.h>
#include <Leadwort/core/math/public/Mat4.h>
#include <Leadwort/core/math/public/Vec3.h>
#include <Leadwort/rendering/bindables/public/Shader.h>
#include <Leadwort/rendering/bindables/public/Texture.h>
#include <Leadwort/rendering/public/GLStateCache.h>
#include <Leadwort/utils/public/Logger.h>

#include <array>

namespace Leadwort::Rendering {

    namespace {
        const Mat4 CAPTURE_PROJECTION { Mat4::PerspectiveLH(90.0f, 1.0f, 0.1f, 10.0f) };

        const std::array<Mat4, 6> CAPTURE_VIEWS {
            Mat4::LookAt(Vec3(0.0f), Vec3( 1.0f,  0.0f,  0.0f), Vec3(0.0f, -1.0f,  0.0f)),
            Mat4::LookAt(Vec3(0.0f), Vec3(-1.0f,  0.0f,  0.0f), Vec3(0.0f, -1.0f,  0.0f)),
            Mat4::LookAt(Vec3(0.0f), Vec3( 0.0f,  1.0f,  0.0f), Vec3(0.0f,  0.0f,  1.0f)),
            Mat4::LookAt(Vec3(0.0f), Vec3( 0.0f, -1.0f,  0.0f), Vec3(0.0f,  0.0f, -1.0f)),
            Mat4::LookAt(Vec3(0.0f), Vec3( 0.0f,  0.0f,  1.0f), Vec3(0.0f, -1.0f,  0.0f)),
            Mat4::LookAt(Vec3(0.0f), Vec3( 0.0f,  0.0f, -1.0f), Vec3(0.0f, -1.0f,  0.0f)),
        };
    }

    IBLBaker::ScratchFBO::ScratchFBO(const int width, const int height) {
        glGenFramebuffers(1, &fbo);
        glGenRenderbuffers(1, &rbo);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    IBLBaker::ScratchFBO::~ScratchFBO() {
        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rbo);
    }

    void IBLBaker::ScratchFBO::AttachCubeFace(const GLuint cubemapID, const int face, const int mip) const {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, cubemapID, mip
        );
    }

    IBLBaker::Result IBLBaker::Bake(const Bindables::Texture& equirectangularHDRI) {
        LW_LOG("IBLBaker: baking IBL textures...");

        Result result{};

        result.EnvironmentCubemap = EquirectToCubemap(equirectangularHDRI, 512);
        result.IrradianceMap      = ConvolveIrradiance(*result.EnvironmentCubemap, 32);
        result.PrefilterMap       = PrefilterEnvironment(*result.EnvironmentCubemap, 128, 5);
        result.BrdfLUT            = GenerateBrdfLUT(512);

        GLStateCache::Get().Invalidate();

        LW_LOG("IBLBaker: done.");

        return result;
    }

    //  Equirect -> Cubemap
    Unique<CubeMapTexture> IBLBaker::EquirectToCubemap(const Bindables::Texture& hdri, const int resolution) {
        auto cubemap { CreateUnique<CubeMapTexture>(resolution, CubeMapTexture::Format::RGB16F, false) };
        const ScratchFBO scratch(resolution, resolution);

        const auto shader { AssetManagement::EngineAssets::GetShader("shaders/ibl/shd_equirect_to_cubemap.glsl") };
        shader->Bind();
        shader->SetUniform("_ProjectionMatrix", CAPTURE_PROJECTION);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdri.GetGpuID());
        shader->SetUniform("_EquirectangularMap", 0);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glViewport(0, 0, resolution, resolution);

        for (int face = 0; face < 6; face++) {
            shader->SetUniform("_ViewMatrix", CAPTURE_VIEWS[face]);
            scratch.AttachCubeFace(cubemap->GetGpuID(), face);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            RenderCube();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        shader->Unbind();

        return cubemap;
    }

    //  Irradiance convolution (diffuse IBL)
    Unique<CubeMapTexture> IBLBaker::ConvolveIrradiance(const CubeMapTexture& envCubemap, const int resolution) {
        auto irradiance { CreateUnique<CubeMapTexture>(resolution, CubeMapTexture::Format::RGB16F, false) };
        const ScratchFBO scratch(resolution, resolution);

        const auto shader { AssetManagement::EngineAssets::GetShader("shaders/ibl/shd_irradiance_convolve.glsl") };
        shader->Bind();
        shader->SetUniform("_ProjectionMatrix", CAPTURE_PROJECTION);

        envCubemap.BindAsInput(0);
        shader->SetUniform("_EnvironmentMap", 0);

        glViewport(0, 0, resolution, resolution);

        for (int face = 0; face < 6; face++) {
            shader->SetUniform("_ViewMatrix", CAPTURE_VIEWS[face]);
            scratch.AttachCubeFace(irradiance->GetGpuID(), face);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            RenderCube();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        shader->Unbind();

        return irradiance;
    }

    //  Specular prefilter (GGX importance sampling, por mip)
    Unique<CubeMapTexture> IBLBaker::PrefilterEnvironment(const CubeMapTexture& envCubemap, const int resolution, const int mipLevels) {
        auto prefilter { CreateUnique<CubeMapTexture>(resolution, CubeMapTexture::Format::RGB16F, true) };
        const ScratchFBO scratch(resolution, resolution);

        const auto shader { AssetManagement::EngineAssets::GetShader("shaders/ibl/shd_prefilter_env.glsl") };
        shader->Bind();
        shader->SetUniform("_ProjectionMatrix", CAPTURE_PROJECTION);

        envCubemap.BindAsInput(0);
        shader->SetUniform("_EnvironmentMap", 0);

        for (int mip = 0; mip < mipLevels; mip++) {
            const int mipRes = resolution >> mip;
            const float roughness = static_cast<float>(mip) / static_cast<float>(mipLevels - 1);

            glViewport(0, 0, mipRes, mipRes);
            shader->SetUniform("_Roughness", roughness);

            for (int face = 0; face < 6; face++) {
                shader->SetUniform("_ViewMatrix", CAPTURE_VIEWS[face]);
                scratch.AttachCubeFace(prefilter->GetGpuID(), face, mip);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                RenderCube();
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        shader->Unbind();

        return prefilter;
    }

    //  BRDF LUT (2D, integración analítica de Fresnel/geometry)
    GLuint IBLBaker::GenerateBrdfLUT(const int resolution) {
        GLuint lutID{};

        glGenTextures(1, &lutID);
        glBindTexture(GL_TEXTURE_2D, lutID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, resolution, resolution, 0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        const ScratchFBO scratch(resolution, resolution);

        glBindFramebuffer(GL_FRAMEBUFFER, scratch.fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lutID, 0);

        LW_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
            "IBLBaker: BRDF LUT framebuffer incomplete.");

        glViewport(0, 0, resolution, resolution);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto shader { AssetManagement::EngineAssets::GetShader("shaders/ibl/shd_brdf_lut.glsl") };
        shader->Bind();
        RenderFullscreenQuad();
        shader->Unbind();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return lutID;
    }

    void IBLBaker::RenderCube() {
        static GLuint vao = 0, vbo = 0;

        if (vao == 0) {
            constexpr float vertices[] = {
                -1,-1,-1,  1,-1,-1,  1, 1,-1,  1, 1,-1, -1, 1,-1, -1,-1,-1,
                -1,-1, 1,  1,-1, 1,  1, 1, 1,  1, 1, 1, -1, 1, 1, -1,-1, 1,
                -1, 1, 1, -1, 1,-1, -1,-1,-1, -1,-1,-1, -1,-1, 1, -1, 1, 1,
                 1, 1, 1,  1, 1,-1,  1,-1,-1,  1,-1,-1,  1,-1, 1,  1, 1, 1,
                -1,-1,-1,  1,-1,-1,  1,-1, 1,  1,-1, 1, -1,-1, 1, -1,-1,-1,
                -1, 1,-1,  1, 1,-1,  1, 1, 1,  1, 1, 1, -1, 1, 1, -1, 1,-1,
            };

            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);

            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

            glBindVertexArray(0);
        }

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    void IBLBaker::RenderFullscreenQuad() {
        static GLuint vao = 0, vbo = 0;

        if (vao == 0) {
            constexpr float vertices[] = {
                // pos               uv
                -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
                 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
                -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
                 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
                 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
            };

            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);

            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

            glBindVertexArray(0);
        }

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

}