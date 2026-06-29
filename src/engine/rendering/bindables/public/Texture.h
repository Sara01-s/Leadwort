#pragma once

#include "../../../core/math/public/Color.h"
#include "Bindable.h"
#include "engine/asset-management/private/AssetKey.h"

#include <glad/glad.h>
#include <string>

namespace Engine::AssetManagement {
	class AssetManager;
}

namespace Engine::Rendering::Bindables {

class Texture final : public Bindable {
public:
	explicit Texture(AssetManagement::AssetKey<Texture>);
	Texture() = delete;
	~Texture() override;

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	Texture(Texture&&) = delete;
	Texture& operator=(Texture&&) = delete;

	void Bind(int slot = 0) const;
	void Bind() const noexcept override;
	void Unbind() const noexcept override;

	[[nodiscard]] int GetWidth() const noexcept { return m_Width; }
	[[nodiscard]] int GetHeight() const noexcept { return m_Height; }
	[[nodiscard]] int GetChannelCount() const noexcept { return m_Channels; }

private:
	friend class AssetManagement::AssetManager;

	void UploadRGBA(const uint8_t* pixels, int width, int height, bool generateMipmaps, bool anisotropicFiltering);
	static void ApplySamplerParams(bool generateMipmaps, bool anisotropicFiltering);

	int m_Width    = 1;
	int m_Height   = 1;
	int m_Channels = 4;
};

} // namespace Engine::Rendering::Bindables