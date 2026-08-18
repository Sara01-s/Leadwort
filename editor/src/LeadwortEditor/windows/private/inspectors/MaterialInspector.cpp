#include "LeadwortEditor/windows/public/inspectors/MaterialInspector.h"

#include "Leadwort/asset-management/public/AssetDatabase.h"
#include "Leadwort/rendering/bindables/public/Material.h"

#include <cstring>
#include <filesystem>
#include <imgui.h>
#include <string_view>

namespace Editor::Windows {

bool DrawTextureSlot(const char* label, Leadwort::Shared<Leadwort::Rendering::Bindables::Texture>& texture, Leadwort::Vec4& st) {
    bool changed { false };
    ImGui::PushID(label);

    ImGui::Text("%s", label);

    constexpr float thumbSize { 64.0f };
    ImGui::BeginChild("##slot", ImVec2(0, thumbSize + 8), true, ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::BeginGroup();
        if (texture) {
            const ImTextureID texID = static_cast<ImTextureID>(static_cast<intptr_t>(texture->GetGpuID()));
            ImGui::Image(texID, ImVec2(thumbSize, thumbSize));
        }
        else {
            const ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(
                p0, { p0.x + thumbSize, p0.y + thumbSize }, IM_COL32(40, 40, 40, 255)
            );
            ImGui::GetWindowDrawList()->AddText(
                { p0.x + 8, p0.y + thumbSize * 0.5f - 8 }, IM_COL32(140, 140, 140, 255), "No Tex"
            );
            ImGui::Dummy(ImVec2(thumbSize, thumbSize));
        }
        ImGui::EndGroup();

        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::PushItemWidth(90.0f);

        float tiling[2] { st.x, st.y };
        if (ImGui::DragFloat2("Tiling", tiling, 0.01f, 0.0f, 0.0f, "%.2f")) {
            st.x = tiling[0];
            st.y = tiling[1];
            changed = true;
        }

        float offset[2] { st.z, st.w };
        if (ImGui::DragFloat2("Offset", offset, 0.01f, 0.0f, 0.0f, "%.2f")) {
            st.z = offset[0];
            st.w = offset[1];
            changed = true;
        }

        ImGui::PopItemWidth();
        ImGui::EndGroup();
    }

    ImGui::EndChild();

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload { ImGui::AcceptDragDropPayload("ASSET_TEXTURE") }) {
            const auto* path { static_cast<const std::filesystem::path*>(payload->Data) };

            texture = Leadwort::AssetManagement::EngineAssets::GetTexture(path->string());
            changed = true;
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::BeginPopupContextItem("##slotContext")) {
        if (ImGui::MenuItem("Clear")) {
            texture = nullptr;
            changed = true;
        }
        ImGui::EndPopup();
    }

    ImGui::PopID();
    ImGui::Spacing();

    return changed;
}

std::string PrettifyUniformName(const std::string_view rawName) {
    std::string name { rawName };

    if (name.starts_with("u_")) {
        name = name.substr(2);
    }

    for (const char* suffix : { "Map", "Sampler", "Tex" }) {
        if (name.ends_with(suffix)) {
            name = name.substr(0, name.size() - std::strlen(suffix));
            break;
        }
    }

    // PascalCase -> "Pascal Case"
    std::string result{};
    for (size_t i = 0; i < name.size(); i++) {
        if (i > 0 && std::isupper(static_cast<unsigned char>(name[i]))) {
            result += ' ';
        }

        result += name[i];
    }

	std::erase(result, '_');

    return result.empty() ? std::string{ rawName } : result;
}

void DrawMaterialInspector(Leadwort::Rendering::Bindables::Material& material) {
    using namespace Leadwort::Rendering::Bindables;

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Texture Maps", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto samplers = material.GetShader().GetSamplers();
        std::vector<std::pair<std::string, Shader::SamplerInfo>> sorted(samplers.begin(), samplers.end());
        std::ranges::sort(sorted, [](const auto& a, const auto& b) { return a.first < b.first; });

    	for (const auto& [uniformName, info] : sorted) {
    		if (info.Type != GL_SAMPLER_2D) {
    			continue;
    		}

    		const auto& textures = material.GetTextures();
    		const auto it = textures.find(uniformName);

    		Leadwort::Shared<Texture> currentTex = (it != textures.end()) ? it->second.Texture : nullptr;
    		Leadwort::Vec4 currentST = (it != textures.end()) ? it->second.ST : Leadwort::Vec4(1.0f, 1.0f, 0.0f, 0.0f);

    		if (DrawTextureSlot(PrettifyUniformName(uniformName).c_str(), currentTex, currentST)) {
    			material.SetTexture(uniformName, currentTex);
    			material.SetTextureST(uniformName, currentST);
    		}
    	}
    }

    if (ImGui::CollapsingHeader("Surface", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto floats = material.GetFloats();
        std::vector<std::pair<std::string, float>> sortedFloats(floats.begin(), floats.end());
        std::ranges::sort(sortedFloats, [](const auto& a, const auto& b) { return a.first < b.first; });

        for (const auto& [uniformName, value] : sortedFloats) {
            float v { value };
            if (ImGui::SliderFloat(PrettifyUniformName(uniformName).c_str(), &v, 0.0f, 1.0f)) {
                material.SetFloat(uniformName, v);
            }
        }
    }
}

} // namespace Editor::Windows