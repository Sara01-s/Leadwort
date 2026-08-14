#pragma once

#include "Leadwort/rendering/bindables/public/Material.h"

#include <string>

namespace Editor::Windows {

bool DrawTextureSlot(const char* label, Leadwort::Shared<Leadwort::Rendering::Bindables::Texture>& texture);
std::string PrettifyUniformName(std::string_view rawName);
void DrawMaterialInspector(Leadwort::Rendering::Bindables::Material& material);

} // namespace Editor::Windows