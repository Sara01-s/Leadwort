#pragma once

#include "Bindable.h"
#include "engine/utils/public/Color.h"
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <string>

namespace Engine::Rendering::Bindables {

class Texture : public Bindable {
public:
	~Texture() override;

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	// Factory methods
	static Texture* FromPath(const std::string& path);
	static Texture* FromMemory(const uint8_t* data, size_t size);
	static Texture* FromRGBA(int width, int height, const uint8_t* rgbaPixels);
	static Texture* CreateSolid(const Utils::Color& color);
	static Texture* CreateEmpty();

	void Bind(int slot = 0) const;
	void Bind()   const noexcept override;
	void Unbind() const noexcept override;

	[[nodiscard]] int GetWidth()    const { return m_Width; }
	[[nodiscard]] int GetHeight()   const { return m_Height; }
	[[nodiscard]] int GetChannels() const { return m_Channels; }

private:
	Texture() = default;

	void LoadFromPath(const std::string& path);
	void UploadRGBA(const uint8_t* pixels, int width, int height, bool mips, bool aniso);
	static void ApplySamplerParams(bool mips, bool aniso);

	int m_Width    = 1;
	int m_Height   = 1;
	int m_Channels = 4;
};

} // namespace Engine::Rendering::Bindables