#pragma once
#include "LeadwortEditor/core/public/IEditorWindow.h"
#include "imgui.h"

#include <format>

namespace Editor::Core {

	class ToolsWindow final : public IEditorWindow {
	public:
		std::string_view GetName() const noexcept override {
			return "Tools";
		}

		void OnGuiRender() override {
			ImGui::Begin("Status");


			
			ImGui::End();
		}
	};

} // namespace Editor::Core