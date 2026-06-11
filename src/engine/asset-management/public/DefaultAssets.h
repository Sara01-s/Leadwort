#pragma once

#include "engine/asset-management/public/AssetManager.h"
#include "engine/rendering/bindables/public/Material.h"
#include "engine/rendering/bindables/public/Shader.h"
#include "engine/rendering/bindables/public/Texture.h"
#include <memory>

namespace Engine::AssetManagement {

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