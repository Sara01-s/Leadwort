#pragma once
#include "../../core/public/IEditorWindow.h"
#include "LeadwortEditor/data/EditorContext.h"
#include "imgui.h"
#include <Leadwort/rendering/public/ScenePicker.h>
#include <Leadwort/systems/public/CameraSystem.h>
#include <Leadwort/systems/public/Input.h>
#include <Leadwort/systems/public/SceneSystem.h>

namespace Editor::Windows {

	class SceneViewport final : public Core::IEditorWindow {
	public:
		using ResizeCallback = std::function<void(int, int)>;

		explicit SceneViewport(Leadwort::Rendering::RG::RenderTexture* sceneRenderTexture, const ResizeCallback& onResize, EditorContext& editorContext)
			: m_SceneRenderTexture(sceneRenderTexture), m_OnResize(onResize), m_EditorContext(editorContext)
		{
			ImGuizmo::SetOrthographic(false);
		}

		std::string_view GetName() const noexcept override { return "Scene"; }

		void ApplyPendingResize() {
			if (m_PendingResize) {
				if (m_OnResize) {
					m_OnResize(m_PendingWidth, m_PendingHeight);
				}
				else {
					m_SceneRenderTexture->Resize(m_PendingWidth, m_PendingHeight);
				}

				m_PendingResize = false;
			}
		}

		void OnGuiRender() override {
			ImGui::Begin(GetName().data());

			const bool isHovered { ImGui::IsWindowHovered() };
			Leadwort::Systems::Input::Mouse::SetViewportHovered(isHovered);

			const ImVec2 availSize { ImGui::GetContentRegionAvail() };
			const ImVec2 renderSize { availSize };

			const ImVec2 imagePos { ImGui::GetCursorScreenPos() };

			const int newWidth { static_cast<int>(renderSize.x) };
			const int newHeight { static_cast<int>(renderSize.y) };

			if (newWidth > 0 && newHeight > 0) {
				const bool resized{ newWidth != m_SceneRenderTexture->GetWidth()
									|| newHeight != m_SceneRenderTexture->GetHeight() };

				if (resized) {
					m_PendingResize = true;
					m_PendingWidth = newWidth;
					m_PendingHeight = newHeight;
				}

				ImGui::Image(m_SceneRenderTexture->GetGpuID(), renderSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

				// Guizmo
				bool guizmoActive { false };
				if (const auto* entityID { std::get_if<Leadwort::EntityID>(&m_EditorContext.Selection) }) {
					ImGuizmo::SetDrawlist();
					ImGuizmo::SetRect(imagePos.x, imagePos.y, renderSize.x, renderSize.y);

					DrawGuizmo(*entityID);

					guizmoActive = ImGuizmo::IsOver() || ImGuizmo::IsUsing();

					DrawComponentGizmos(*entityID, imagePos, renderSize);
				}
				Leadwort::Systems::Input::Mouse::SetUIHovered(guizmoActive);

				if (isHovered
					&& ImGui::IsMouseClicked(ImGuiMouseButton_Left)
					&& !Leadwort::Systems::Input::Mouse::IsCaptured()
					&& !ImGuizmo::IsOver())
				{
					const ImVec2 mousePos = ImGui::GetMousePos();
					const Leadwort::Vec2 normalizedPoint { (mousePos.x - imagePos.x) / renderSize.x,
														   (mousePos.y - imagePos.y) / renderSize.y };

					HandlePick(normalizedPoint);
				}
			}

			ImGui::End();
		}

	private:
		static bool ProjectToScreen(
			const Leadwort::Vec3& worldPos,
			const Leadwort::Mat4& view,
			const Leadwort::Mat4& projection,
			const ImVec2& viewportPos,
			const ImVec2& viewportSize,
			ImVec2& outScreen
		) {
			using namespace Leadwort;

			const Leadwort::Vec4 clip { projection * view * Leadwort::Vec4(worldPos.x, worldPos.y, worldPos.z, 1.0f) };

			if (clip.w <= 0.0001f) {
				return false;
			}

			const Vec3 ndc { Vec3(clip.x, clip.y, clip.z) / clip.w };

			outScreen.x = viewportPos.x + (ndc.x * 0.5f + 0.5f) * viewportSize.x;
			outScreen.y = viewportPos.y + (1.0f - (ndc.y * 0.5f + 0.5f)) * viewportSize.y;

			return true;
		}

		void DrawGuizmo(const Leadwort::EntityID entityID) const {
			using namespace Leadwort;

			const auto* scene { Systems::SceneSystem::Get().GetCurrentScene() };
			const auto* sceneCamera { Systems::CameraSystem::Get().GetSceneCamera() };
			if (!scene || !sceneCamera) {
				return;
			}

			const auto& entity { scene->GetEntity(entityID) };

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::Enable(true);

			auto& transform { entity->GetTransform() };
			Mat4 transformMatrix { transform.GetWorldMatrix() };

			Mat4 viewMatrix { sceneCamera->GetViewMatrix() };
			Mat4 projectionMatrix { sceneCamera->GetProjectionMatrix() };

			ImGuizmo::Manipulate(
				viewMatrix.ToPtr(),
				projectionMatrix.ToPtr(),
				m_EditorContext.GizmoOperation,
				m_EditorContext.GizmoMode,
				transformMatrix.ToPtr()
			);

			if (ImGuizmo::IsUsing()) {
				Vec3 translation{}, rotation{}, scale{};
				ImGuizmo::DecomposeMatrixToComponents(transformMatrix.ToPtr(), translation.ToPtr(), rotation.ToPtr(), scale.ToPtr());
				transform.SetLocalFromWorld(translation, rotation, scale);
			}
		}

		void HandlePick(const Leadwort::Vec2& normalizedPoint) const {
			using namespace Leadwort;

			const auto* sceneCam { Systems::CameraSystem::Get().GetSceneCamera() };
			if (!sceneCam) {
				return;
			}

			const Ray ray { sceneCam->ScreenPointToRay(normalizedPoint) };

			const auto* scene { Systems::SceneSystem::Get().GetCurrentScene() };
			if (!scene) {
				return;
			}

			const auto pickedEntity { Rendering::ScenePicker::Pick(ray, *scene) };

			if (pickedEntity != Leadwort::Core::Entity::ROOT_ENTITY_ID) {
				m_EditorContext.Selection = pickedEntity;
			}
			else {
				m_EditorContext.ClearSelection();
			}
		}

		static void DrawComponentGizmos(const Leadwort::EntityID entityID, const ImVec2& viewportPos, const ImVec2& viewportSize) {
			using namespace Leadwort;

			const auto* scene { Systems::SceneSystem::Get().GetCurrentScene() };
			const auto* sceneCamera { Systems::CameraSystem::Get().GetSceneCamera() };
			if (!scene || !sceneCamera) {
				return;
			}

			const auto* entity { scene->GetEntity(entityID) };
			if (!entity) {
				return;
			}

			const Mat4 viewMatrix { sceneCamera->GetViewMatrix() };
			const Mat4 projectionMatrix { sceneCamera->GetProjectionMatrix() };

			ImDrawList* drawList { ImGui::GetWindowDrawList() };
			constexpr ImU32 gizmoColor { IM_COL32(255, 220, 80, 255) };

			for (const auto* component : entity->GetAllComponents()) {
				for (const auto& line : component->GetGizmoLines()) {
					ImVec2 screenStart{}, screenEnd{};

					const bool startVisible { ProjectToScreen(line.start, viewMatrix, projectionMatrix, viewportPos, viewportSize, screenStart) };
					const bool endVisible   { ProjectToScreen(line.end,   viewMatrix, projectionMatrix, viewportPos, viewportSize, screenEnd) };

					if (startVisible && endVisible) {
						drawList->AddLine(screenStart, screenEnd, gizmoColor, 1.5f);
					}
				}
			}
		}

	private:
		Leadwort::Rendering::RG::RenderTexture* m_SceneRenderTexture{};
		ResizeCallback m_OnResize{};
		EditorContext& m_EditorContext;

		bool m_PendingResize { false };
		int m_PendingWidth { 0 };
		int m_PendingHeight { 0 };
	};

} // namespace Editor::Windows