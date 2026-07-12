#pragma once
#include "../../core/public/IEditorWindow.h"
#include "imgui.h"
#include "rendering/public/RenderTexture.h"

namespace Editor::Windows {

class SceneViewport final : public Core::IEditorWindow {
public:
	using ResizeCallback = std::function<void(int, int)>;

	explicit SceneViewport(
		Engine::Rendering::RenderTexture* sceneRenderTexture,
		const ResizeCallback& onResize
	) : m_SceneRenderTexture(sceneRenderTexture), m_OnResize(onResize) {}

	constexpr std::string_view GetName() override {
		return "Scene Viewport";
	}

	void OnGuiRender() override {
		ImGui::Begin(GetName().data());

		const ImVec2 availSize = ImGui::GetContentRegionAvail();
		const ImVec2 renderSize = availSize;

		const int newWidth  = static_cast<int>(renderSize.x);
		const int newHeight = static_cast<int>(renderSize.y);

		if (newWidth > 0 && newHeight > 0) {
			const bool resized {
				newWidth != m_SceneRenderTexture->GetWidth()
				|| newHeight != m_SceneRenderTexture->GetHeight()
			};

			if (resized) {
				if (m_OnResize) {
					m_OnResize(newWidth, newHeight);
				}
				else {
					m_SceneRenderTexture->Resize(newWidth, newHeight);
				}
			}

			ImGui::Image(
			   m_SceneRenderTexture->GetGpuID(),
			   renderSize,
			   /*uv 0*/ ImVec2(0.0f, 1.0f),
			   /*uv 1*/ ImVec2(1.0f, 0.0f)
			);
		}

		ImGui::End();
	}

private:
	Engine::Rendering::RenderTexture* m_SceneRenderTexture{};
	ResizeCallback m_OnResize{};
};

}