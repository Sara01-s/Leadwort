#include "engine/asset-management/public/DefaultAssets.h"

namespace Engine::AssetManagement {

using namespace Bindables;

namespace {
	Shared<Shader>  s_UnlitShader;
	Shared<Shader>  s_LitShader;
	Shared<Texture> s_DefaultTexture;
}

Shared<Shader> DefaultAssets::GetUnlitShader() {
	if (!s_UnlitShader) {
		s_UnlitShader = EngineAssets::GetShader("shaders/shd_unlit.glsl");
	}

	return s_UnlitShader;
}

Shared<Shader> DefaultAssets::GetLitShader() {
	if (!s_LitShader) {
		 s_LitShader = EngineAssets::GetShader("shaders/shd_lit.glsl");
	}

	return s_LitShader;
}

Shared<Material> DefaultAssets::CreateUnlitMaterial() {
	return EngineAssets::CreateMaterial(GetUnlitShader());
}

Shared<Material> DefaultAssets::CreateLitMaterial() {
	auto material = EngineAssets::CreateMaterial(GetLitShader());

	material->SetColor4("_Color", Utils::Color::White());
	material->SetTexture("_DiffuseTexture", GetTexture(), 0);
	material->SetFloat("_RoughnessIntensity", 0.5f);
	material->SetFloat("_MetallicIntensity",  0.0f);

	return material;
}

Shared<Texture> DefaultAssets::GetTexture() {
	if (!s_DefaultTexture) {
		s_DefaultTexture = EngineAssets::GetTexture("textures/tex_white_1x1.png");
	}

	return s_DefaultTexture;
}

} // namespace Engine::AssetManagement