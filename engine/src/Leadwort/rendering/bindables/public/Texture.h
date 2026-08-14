#pragma once

#include "IBindable.h"
#include <Leadwort/asset-management/private/AssetKey.h>

#include "glad/glad.h"

namespace Leadwort::AssetManagement {
	class AssetDatabase;
}

namespace Leadwort::Rendering::Bindables {

	class Texture final : public IBindable {
	public:
		explicit Texture(AssetManagement::AssetKey<Texture>, std::string_view name = "Unnamed Texture");
		explicit Texture(std::string_view exrPath, AssetManagement::AssetKey<Texture>);
		Texture() = delete;
		~Texture() override;

		Texture(Texture&&) = delete;
		Texture& operator=(Texture&&) = delete;

		void Bind(int slot = 0) const;
		void Bind() const noexcept override;
		void Unbind() const noexcept override;

		[[nodiscard]] std::string GetName() const noexcept { return m_Name; }
		[[nodiscard]] int GetWidth() const noexcept { return m_Width; }
		[[nodiscard]] int GetHeight() const noexcept { return m_Height; }
		[[nodiscard]] int GetChannelCount() const noexcept { return m_Channels; }
		[[nodiscard]] std::string GetPath() const noexcept { return m_Path; }

	private:
		friend class AssetManagement::AssetDatabase;

		void SetPath(std::string path) { m_Path = std::move(path); }
		void UploadRGBA(const uint8_t* pixels, int width, int height, bool generateMipmaps, bool anisotropicFiltering);
		static void ApplySamplerParams(bool generateMipmaps, bool anisotropicFiltering);

	private:
		std::string m_Name{};
		std::string m_Path{};
		int m_Width    { 1 };
		int m_Height   { 1 };
		int m_Channels { 4 };
	};

} // namespace Engine::Rendering::Bindables