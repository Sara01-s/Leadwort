#pragma once

#include "engine/asset-management/public/AssetManager.h"
#include "engine/rendering/bindables/public/Material.h"
#include "engine/rendering/bindables/public/Shader.h"
#include "engine/rendering/bindables/public/Texture.h"
#include <memory>

namespace Engine::AssetManagement {

class DefaultAssets {
public:
	static std::shared_ptr<Rendering::Bindables::Shader> GetUnlitShader();
	static std::shared_ptr<Rendering::Bindables::Shader> GetLitShader();
	static std::shared_ptr<Rendering::Bindables::Texture> GetTexture();
	static std::shared_ptr<Rendering::Bindables::Texture> GetWhiteTexture();
	static std::shared_ptr<Rendering::Bindables::Material> GetUnlitMaterial();
	static std::shared_ptr<Rendering::Bindables::Material> GetLitMaterial();
};

} // namespace E