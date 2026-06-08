#include "engine/asset-management/public/DefaultAssets.h"

namespace Engine::AssetManagement {

using Shader   = Rendering::Bindables::Shader;
using Texture  = Rendering::Bindables::Texture;
using Material = Rendering::Bindables::Material;
using Color    = Utils::Color;

std::shared_ptr<Shader> DefaultAssets::GetUnlitShader() {
	return EngineAssets::LoadShader("shaders/shd_unlit.glsl");
}

std::shared_ptr<Shader> DefaultAssets::GetLitShader() {
	return EngineAssets::LoadShader("shaders/shd_lit.glsl");
}

std::shared_ptr<Texture> DefaultAssets::GetTexture() {
	return EngineAssets::LoadTexture("textures/tex_square.png");
}

std::shared_ptr<Texture> DefaultAssets::GetWhiteTexture() {
	static auto s_WhiteTexture = std::shared_ptr<Texture>(Texture::CreateSolid(Color::White()));
	return s_WhiteTexture;
}

std::shared_ptr<Material> DefaultAssets::GetUnlitMaterial() {
	auto mat = std::make_shared<Material>(GetUnlitShader());
	mat->SetTexture("_DiffuseTexture", GetTexture().get());
	mat->SetColor4("_Color", Color::White());
	return mat;
}

std::shared_ptr<Material> DefaultAssets::GetLitMaterial() {
	auto mat = std::make_shared<Material>(GetLitShader());
	mat->SetTexture("_DiffuseTexture", GetTexture().get());
	return mat;
}

} // namespace Engine::AssetManagement