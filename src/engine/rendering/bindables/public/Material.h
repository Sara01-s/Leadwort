#pragma once

#include "Bindable.h"
#include "Shader.h"
#include "Texture.h"
#include "engine/core/public/Core.h"
#include "engine/core/math/public/Mat4.h"
#include <glad/glad.h>
#include <string>
#include <unordered_map>

#include <memory>

namespace Engine::Rendering::Bindables {

class CubeMap;

class Material : public Bindable {
public:
	explicit Material(const Shared<Shader>& shader, AssetManagement::AssetKey<Material>);
	Material() = delete;

	explicit Material(Shader&& shader) = delete;
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    // Setters
    void SetInt(const std::string& name, int value);
    void SetFloat(const std::string& name, float value);
    void SetVec3(const std::string& name, const Vec3& value);
    void SetVec4(const std::string& name, const Vec4& value);
    void SetMat3(const std::string& name, const Mat3& value);
    void SetMat4(const std::string& name, const Mat4& value);

    void SetTexture(const std::string& name, const Shared<Texture>& texture);
    void SetTexture(const std::string& name, const Shared<Texture>& texture, int slot);
    void SetTexture(const std::string& name, uint32_t gpuID, int slot, uint32_t target = GL_TEXTURE_2D);

	void SetCubeMap(const std::string& name, const CubeMap* cubeMap);
	void SetCubeMap(const std::string& name, const CubeMap* cubeMap, int slot);

    void SetColor3(const std::string& name, const Utils::Color& color);
    void SetColor4(const std::string& name, const Utils::Color& color);

    void SetMainColor(const Utils::Color& color);
    void SetMainTexture(const Shared<Texture>& texture);

    [[nodiscard]] Shared<Material> Clone() const;

    void Bind()   const noexcept override;
    void Unbind() const noexcept override;

    [[nodiscard]] Shader const& GetShader() const { return *m_Shader; }

private:
    struct TextureSlot {
        Shared<Texture> texture = nullptr;
        uint32_t  gpuID    = 0;
        uint32_t  target   = GL_TEXTURE_2D;
        int       slot     = -1; // -1 = auto
    };

	Shared<Shader> m_Shader;
	mutable uint32_t m_LastShaderVersion { IntInfinity } ;

    std::unordered_map<std::string, int>   m_Ints;
    std::unordered_map<std::string, float> m_Floats;
    std::unordered_map<std::string, Vec3> m_Vec3s;
    std::unordered_map<std::string, Vec4> m_Vec4s;
    std::unordered_map<std::string, Mat3> m_Mat3s;
    std::unordered_map<std::string, Mat4> m_Mat4s;
    std::unordered_map<std::string, TextureSlot> m_Textures;
};

} // namespace Engine::Rendering::Bindables