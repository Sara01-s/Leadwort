#pragma once

#include <string>
#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Bindable.h"
#include "Shader.h"
#include "Texture.h"

#include <memory>

namespace Engine::Rendering::Bindables {

class CubeMap;

class Material : public Bindable {
public:
	explicit Material(const std::shared_ptr<Shader>& shader);
	explicit Material(Shader&& shader);

    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    // Setters
    void SetInt(const std::string& name, int value);
    void SetFloat(const std::string& name, float value);
    void SetVec3(const std::string& name, const glm::vec3& value);
    void SetVec4(const std::string& name, const glm::vec4& value);
    void SetMat3(const std::string& name, const glm::mat3& value);
    void SetMat4(const std::string& name, const glm::mat4& value);

    void SetTexture(const std::string& name, Texture* texture);
    void SetTexture(const std::string& name, Texture* texture, int slot);
    void SetTexture(const std::string& name, uint32_t gpuID, int slot, uint32_t target = GL_TEXTURE_2D);

	void SetCubeMap(const std::string& name, const CubeMap* cubeMap);
	void SetCubeMap(const std::string& name, const CubeMap* cubeMap, int slot);

    void SetColor3(const std::string& name, const Utils::Color& color);
    void SetColor4(const std::string& name, const Utils::Color& color);

    void SetMainColor(const Utils::Color& color);
    void SetMainTexture(Texture* texture);

    [[nodiscard]] std::unique_ptr<Material> Clone() const;

    void Bind()   const noexcept override;
    void Unbind() const noexcept override;

    [[nodiscard]] Shader const& GetShader() const { return *m_Shader; }

private:
    struct TextureSlot {
        Texture*  texture  = nullptr;
        uint32_t  gpuID    = 0;
        uint32_t  target   = GL_TEXTURE_2D;
        int       slot     = -1; // -1 = auto
    };

	std::shared_ptr<Shader> m_Shader;

    std::unordered_map<std::string, int>       m_Ints;
    std::unordered_map<std::string, float>     m_Floats;
    std::unordered_map<std::string, glm::vec3> m_Vec3s;
    std::unordered_map<std::string, glm::vec4> m_Vec4s;
    std::unordered_map<std::string, glm::mat3> m_Mat3s;
    std::unordered_map<std::string, glm::mat4> m_Mat4s;
    std::unordered_map<std::string, TextureSlot> m_Textures;
};

} // namespace Engine::Rendering::Bindables