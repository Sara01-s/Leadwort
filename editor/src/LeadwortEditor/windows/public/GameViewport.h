#pragma once
#include "imgui.h"

namespace Editor::Windows {

	class GameViewport final : public Core::IEditorWindow {
	public:
		using ResizeCallback = std::function<void(int, int)>;

		explicit GameViewport(
		   Leadwort::Rendering::RG::RenderTexture* gameRenderTexture,
		   const ResizeCallback& onResize,
		   const float targetAspectRatio = 16.0f / 9.0f
		) : m_GameRenderTexture(gameRenderTexture), m_OnResize(onResize), m_TargetAspectRatio(targetAspectRatio) {}

		std::string_view GetName() const noexcept override {
			return "Game";
		}

		void ApplyPendingResize() {
			if (m_PendingResize) {
				if (m_OnResize) {
					m_OnResize(m_PendingWidth, m_PendingHeight);
				}
				else {
					m_GameRenderTexture->Resize(m_PendingWidth, m_PendingHeight);
				}
				m_PendingResize = false;
			}
		}

		void OnGuiRender() override {
			ImGui::Begin(GetName().data());

			const ImVec2 availSize = ImGui::GetContentRegionAvail();
			ImVec2 renderSize = availSize;

			if (m_TargetAspectRatio > 0.0f && availSize.x > 0 && availSize.y > 0) {
				const float windowAspect = availSize.x / availSize.y;

				if (windowAspect > m_TargetAspectRatio) {
					renderSize.y = availSize.y;
					renderSize.x = renderSize.y * m_TargetAspectRatio;
				}
				else {
					renderSize.x = availSize.x;
					renderSize.y = renderSize.x / m_TargetAspectRatio;
				}

				const float cursorX = (availSize.x - renderSize.x) * 0.5f;
				const float cursorY = (availSize.y - renderSize.y) * 0.5f;
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + cursorX, ImGui::GetCursorPosY() + cursorY));
			}

			const int newWidth  = static_cast<int>(renderSize.x);
			const int newHeight = static_cast<int>(renderSize.y);

			if (newWidth > 0 && newHeight > 0) {
				const bool resized {
					newWidth != m_GameRenderTexture->GetWidth()
					|| newHeight != m_GameRenderTexture->GetHeight()
				 };

				if (resized) {
					m_PendingResize = true;
					m_PendingWidth  = newWidth;
					m_PendingHeight = newHeight;
				}

				ImGui::Image(
				   m_GameRenderTexture->GetGpuID(),
				   renderSize,
				   ImVec2(0.0f, 1.0f),
				   ImVec2(1.0f, 0.0f)
				);
			}

			ImGui::End();

			ImGui::Begin("Game Aspect Ratio");
			if (ImGui::BeginCombo("##", m_TargetAspectRatio == -1.0f ? "Free" : (m_TargetAspectRatio > 1.5f ? "16:9" : "4:3"))) {
				if (ImGui::Selectable("Free Aspect")) { m_TargetAspectRatio = -1.0f; }
				if (ImGui::Selectable("16:9"))        { m_TargetAspectRatio = 16.0f / 9.0f; }
				if (ImGui::Selectable("4:3"))         { m_TargetAspectRatio = 4.0f / 3.0f; }
				ImGui::EndCombo();
			}
			ImGui::End();
		}

		void SetTargetAspectRatio(const float targetAspectRatio) noexcept { m_TargetAspectRatio = targetAspectRatio; }

	private:
		Leadwort::Rendering::RG::RenderTexture* m_GameRenderTexture{};
		ResizeCallback m_OnResize{};
		float m_TargetAspectRatio{};

		bool m_PendingResize { false };
		int  m_PendingWidth { 0 };
		int  m_PendingHeight { 0 };
	};

}