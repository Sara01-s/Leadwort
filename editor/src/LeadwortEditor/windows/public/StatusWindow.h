#pragma once
#include "LeadwortEditor/core/public/IEditorWindow.h"
#include "imgui.h"

#include <format>

namespace Editor::Core {

class StatusWindow final : public IEditorWindow {
public:
	std::string_view GetName() const noexcept override {
		return "Status";
	}

	void OnGuiRender() override {
		ImGui::Begin("Status");
		ImGui::Separator();

		ImGui::Text("Status");

		const float frameRate = ImGui::GetIO().Framerate;
		const float frameTime = 1000.0f / frameRate;

		ImGui::Text("%s", std::format("FPS: {:.1f}", frameRate).c_str());
		ImGui::Text("%s", std::format("Frame Time: {:.3f} ms", frameTime).c_str());

		ImGui::End();
	}
};

} // namespace Editor::Core