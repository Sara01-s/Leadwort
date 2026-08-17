#pragma once

#include "../../../../../engine/src/Leadwort/events/public/Event.h"
#include "LeadwortEditor/data/EditorContext.h"
#include "imgui.h"

#include <LeadwortEditor/core/public/IEditorWindow.h>

namespace Editor::Windows {

	class ConsoleWindow final : public Core::IEditorWindow {
	public:
		explicit ConsoleWindow(EditorContext& editorContext)
			: m_EditorContext(editorContext)
		{
			m_LogBuffer.reserve(m_EditorContext.LogHistory.size());
			for (const auto& historicalMessage : m_EditorContext.LogHistory) {
				m_LogBuffer.push_back(CleanAnsiCodes(historicalMessage));
			}

			m_LogToken = m_EditorContext.LogCallback.Subscribe([this](const std::string& message) {
				m_LogBuffer.push_back(CleanAnsiCodes(message));
				m_ScrollToBottom = true;
			});
		}

		~ConsoleWindow() override {
			m_EditorContext.LogCallback.Unsubscribe(m_LogToken);
		}

		std::string_view GetName() const noexcept override { return "Console"; }

		void OnGuiRender() override {
			ImGui::Begin(GetName().data());

			if (ImGui::Button("Clear")) {
				m_LogBuffer.clear();
			}

			ImGui::SameLine();

			if (ImGui::Button("Copy")) {
				std::string clipboardContent{};
				size_t totalSize { 0 };

				for (const auto& log : m_LogBuffer) {
					totalSize += log.size() + 1;
				}

				clipboardContent.reserve(totalSize);

				for (const auto& log : m_LogBuffer) {
					clipboardContent += log;
					clipboardContent += '\n';
				}

				ImGui::SetClipboardText(clipboardContent.c_str());
			}

			ImGui::SameLine();
			ImGui::TextDisabled("| Total Entries: %zu", m_LogBuffer.size());

			ImGui::Separator();

			const float footerHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
			ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footerHeight), false, ImGuiWindowFlags_HorizontalScrollbar);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));

			for (const auto& log : m_LogBuffer) {
				auto textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

				if (log.find("[ERROR]") != std::string::npos) {
					textColor = ImVec4(1.0f, 0.33f, 0.33f, 1.0f);
				}
				else if (log.find("[WARNING]") != std::string::npos) {
					textColor = ImVec4(1.0f, 0.79f, 0.28f, 1.0f);
				}
				else if (log.find("[INFO]") != std::string::npos) {
					textColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
				}

				ImGui::TextColored(textColor, "%s", log.c_str());
			}

			ImGui::PopStyleVar();

			if (m_ScrollToBottom) {
				ImGui::SetScrollHereY(1.0f);
				m_ScrollToBottom = false;
			}

			ImGui::EndChild();

			ImGui::End();
		}

	private:
		static std::string CleanAnsiCodes(const std::string& input) {
			std::string cleanString{};
			cleanString.reserve(input.size());

			for (size_t i = 0; i < input.size(); ++i) {
				if (input[i] == '\033' && i + 1 < input.size() && input[i + 1] == '[') {
					while (i < input.size() && input[i] != 'm') {
						++i;
					}

					continue;
				}

				cleanString += input[i];
			}

			return cleanString;
		}

	private:
		EditorContext& m_EditorContext;
		Leadwort::Events::Token m_LogToken{};
		std::vector<std::string> m_LogBuffer{};
		bool m_ScrollToBottom { false };
	};

}