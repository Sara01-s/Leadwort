#pragma once
#include "../../core/public/IEditorWindow.h"
#include "LeadwortEditor/data/EditorContext.h"
#include "SceneTools.h"
#include "imgui.h"
#include <Leadwort/rendering/public/CoordinateSystem.h>
#include <Leadwort/rendering/public/ScenePicker.h>
#include <Leadwort/systems/public/CameraSystem.h>
#include <Leadwort/systems/public/Input.h>
#include <Leadwort/systems/public/SceneSystem.h>

namespace Editor::Windows {

	class SceneViewport final : public Core::IEditorWindow {
	public:
		using ResizeCallback = std::function<void(int, int)>;

		explicit SceneViewport(Leadwort::Rendering::RG::RenderTexture* sceneRenderTexture, const ResizeCallback& onResize, EditorContext& editorContext)
			: m_SceneRenderTexture(sceneRenderTexture), m_OnResize(onResize), m_EditorContext(editorContext), m_SceneTools(editorContext)
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

			// Hovered *or* focused, counting the tools overlay: the shortcuts should keep
			// working right after clicking a tool, with the pointer still over it.
			const bool ownsKeyboard { ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)
									  || ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) };
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

				// Gizmo tools, overlaid on the top-left corner of the rendered image.
				// Child windows draw above their parent's content, so this stays on top
				// of both the image and the gizmos regardless of submission order.
				m_SceneTools.Draw(imagePos, ownsKeyboard);
				const bool toolsHovered { m_SceneTools.IsHovered() };

				// Component gizmos for every entity in the scene (frustums, light shapes, ...).
				if (m_EditorContext.ShowGizmos) {
					DrawAllComponentGizmos(imagePos, renderSize);
				}

				// Guizmo
				bool guizmoActive { false };
				if (const auto* entityID { std::get_if<Leadwort::EntityID>(&m_EditorContext.Selection) }) {
					ImGuizmo::SetDrawlist();
					ImGuizmo::SetRect(imagePos.x, imagePos.y, renderSize.x, renderSize.y);

					DrawGuizmo(*entityID);

					guizmoActive = ImGuizmo::IsOver() || ImGuizmo::IsUsing();
				}
				Leadwort::Systems::Input::Mouse::SetUIHovered(guizmoActive || toolsHovered);

				if (isHovered
					&& !toolsHovered
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
		static ImU32 ToImU32(const Leadwort::Color& color) {
			const Leadwort::Color c { color.Clamped() };
			return IM_COL32(
				static_cast<int>(c.r * 255.0f + 0.5f),
				static_cast<int>(c.g * 255.0f + 0.5f),
				static_cast<int>(c.b * 255.0f + 0.5f),
				static_cast<int>(c.a * 255.0f + 0.5f)
			);
		}

		static ImVec2 ClipToScreen(
			const Leadwort::Vec4& clip,
			const ImVec2& viewportPos,
			const ImVec2& viewportSize
		) {
			const float invW { 1.0f / clip.w };
			const float ndcX { clip.x * invW };
			const float ndcY { clip.y * invW };

			return ImVec2 {
				viewportPos.x + (ndcX * 0.5f + 0.5f) * viewportSize.x,
				viewportPos.y + (1.0f - (ndcY * 0.5f + 0.5f)) * viewportSize.y
			};
		}

		static bool ClipSegmentToScreen(
			const Leadwort::Vec3& p0,
			const Leadwort::Vec3& p1,
			const Leadwort::Mat4& viewProjection,
			const ImVec2& viewportPos,
			const ImVec2& viewportSize,
			ImVec2& outA,
			ImVec2& outB
		) {
			constexpr float wEps { 1e-4f };

			Leadwort::Vec4 c0 { viewProjection * Leadwort::Vec4(p0.x, p0.y, p0.z, 1.0f) };
			Leadwort::Vec4 c1 { viewProjection * Leadwort::Vec4(p1.x, p1.y, p1.z, 1.0f) };

			const bool behind0 { c0.w <= wEps };
			const bool behind1 { c1.w <= wEps };

			if (behind0 && behind1) {
				return false;
			}

			if (behind0 != behind1) {
				// Clip space is linear in the segment parameter, so lerping to the
				// point where w == wEps gives the exact near-plane intersection.
				const float t { (wEps - c0.w) / (c1.w - c0.w) };
				const Leadwort::Vec4 crossing { c0.Lerp(c1, t) };

				if (behind0) {
					c0 = crossing;
				}
				else {
					c1 = crossing;
				}
			}

			outA = ClipToScreen(c0, viewportPos, viewportSize);
			outB = ClipToScreen(c1, viewportPos, viewportSize);

			return true;
		}

		void DrawGuizmo(const Leadwort::EntityID entityID) const {
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

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::Enable(true);

			auto& transform { entity->GetTransform() };

			Mat4 gizmoMatrix { transform.GetWorldMatrix() };
			Mat4 viewMatrix { Rendering::CoordinateSystem::GizmoViewMatrix(sceneCamera->GetViewMatrix()) };
			Mat4 projectionMatrix { Rendering::CoordinateSystem::GizmoProjectionMatrix(sceneCamera->GetProjectionMatrix()) };

			const bool manipulated {
				ImGuizmo::Manipulate(
					viewMatrix.ToPtr(),
					projectionMatrix.ToPtr(),
					m_EditorContext.GizmoOperation,
					m_EditorContext.GizmoMode,
					gizmoMatrix.ToPtr()
				)
			};

			if (manipulated) {
				transform.SetWorldMatrix(gizmoMatrix);
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

		static void DrawAllComponentGizmos(const ImVec2& viewportPos, const ImVec2& viewportSize) {
			using namespace Leadwort;

			const auto* scene { Systems::SceneSystem::Get().GetCurrentScene() };
			const auto* sceneCamera { Systems::CameraSystem::Get().GetSceneCamera() };
			if (!scene || !sceneCamera) {
				return;
			}

			const Mat4 viewProjection { sceneCamera->GetProjectionMatrix() * sceneCamera->GetViewMatrix() };

			ImDrawList* drawList { ImGui::GetWindowDrawList() };
			constexpr ImU32 defaultGizmoColor { IM_COL32(255, 220, 80, 255) };

			for (const auto& entity: scene->GetEntityMap() | std::views::values) {
				if (!entity) {
					continue;
				}

				for (const auto* component : entity->GetAllComponents()) {
					for (const auto& line : component->GetGizmoLines()) {
						ImVec2 screenStart{}, screenEnd{};

						if (ClipSegmentToScreen(line.start, line.end, viewProjection, viewportPos, viewportSize, screenStart, screenEnd)) {
							const ImU32 lineColor { line.color ? ToImU32(*line.color) : defaultGizmoColor };
							drawList->AddLine(screenStart, screenEnd, lineColor, 1.5f);
						}
					}
				}
			}
		}

	private:
		Leadwort::Rendering::RG::RenderTexture* m_SceneRenderTexture{};
		ResizeCallback m_OnResize{};
		EditorContext& m_EditorContext;
		SceneTools m_SceneTools;

		bool m_PendingResize { false };
		int m_PendingWidth { 0 };
		int m_PendingHeight { 0 };
	};

} // namespace Editor::Windows