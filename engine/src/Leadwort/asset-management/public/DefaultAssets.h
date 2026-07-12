#pragma once

#include <Leadwort/asset-management/public/AssetManager.h>
#include <Leadwort/rendering/bindables/public/Material.h>
#include <Leadwort/rendering/bindables/public/Shader.h>
#include <Leadwort/rendering/bindables/public/Texture.h>
#include <memory>

namespace Leadwort::AssetManagement {

namespace Bindables = Rendering::Bindables;

class DefaultAssets {
public:
	static Shared<Bindables::Shader>   GetUnlitShader();
	static Shared<Bindables::Shader>   GetLitShader();
	static Shared<Bindables::Texture>  GetTexture();
	static Shared<Bindables::Material> CreateUnlitMaterial();
	static Shared<Bindables::Material> CreateLitMaterial();
};

} // namespace Engine::AssetManagement