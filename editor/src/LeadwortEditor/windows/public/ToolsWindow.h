#pragma once

#include "LeadwortEditor/core/public/IEditorWindow.h"
#include "LeadwortEditor/data/EditorContext.h"
#include "Leadwort/serialization/SceneSerializer.h"
#include "Leadwort/systems/public/SceneSystem.h"
#include "imgui.h"
#include <nfd.hpp>

#include <filesystem>

namespace Editor::Core {

    class ToolsWindow final : public IEditorWindow {
    public:
        explicit ToolsWindow(EditorContext& editorContext)
            : m_EditorContext(editorContext) {}

        std::string_view GetName() const noexcept override {
            return "Tools";
        }

        void OnGuiRender() override {
            if (ImGui::BeginMainMenuBar()) {
                DrawFileMenu();
                DrawEditMenu();
                DrawEntitiesMenu();
                ImGui::EndMainMenuBar();
            }
        }

    private:
        void DrawFileMenu() {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Scene")) {
                    Leadwort::Systems::SceneSystem::Get().LoadDefaultScene();
                    m_EditorContext.OpenedScenePath.clear();
                    m_EditorContext.ClearSelection();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Open Scene...")) {
                    OpenSceneDialog();
                }

                const auto* currentScene { Leadwort::Systems::SceneSystem::Get().GetCurrentScene() };
                const bool hasScene { currentScene != nullptr };
                const bool hasPath  { !m_EditorContext.OpenedScenePath.empty() };

                if (ImGui::MenuItem("Save Scene", nullptr, false, hasScene && hasPath)) {
                    SaveScene(m_EditorContext.OpenedScenePath);
                }

                if (ImGui::MenuItem("Save Scene As...", nullptr, false, hasScene)) {
                    SaveSceneAsDialog();
                }

                ImGui::EndMenu();
            }
        }

        static void DrawEditMenu() {
            if (ImGui::BeginMenu("Edit")) {
                ImGui::TextDisabled("Nothing here yet");
                ImGui::EndMenu();
            }
        }

        static void DrawEntitiesMenu() {
            if (ImGui::BeginMenu("Entities")) {
                ImGui::TextDisabled("Nothing here yet");
                ImGui::EndMenu();
            }
        }

        // Filtro reutilizable: solo .json
        static constexpr nfdu8filteritem_t kSceneFilter[1] { { "Scene (JSON)", "json" } };

        void OpenSceneDialog() {
            NFD::UniquePathU8 outPath{};

            const nfdresult_t result { NFD::OpenDialog(outPath, kSceneFilter, 1) };

            if (result == NFD_OKAY) {
                OpenScene(std::filesystem::path(outPath.get()));
            }
            else if (result == NFD_ERROR) {
                LW_ERROR("ToolsWindow: NFD OpenDialog error: ", NFD::GetError());
            }
            // NFD_CANCEL: el usuario cerró el diálogo sin elegir nada, no hacemos nada
        }

        void SaveSceneAsDialog() {
            NFD::UniquePathU8 outPath{};

            const nfdresult_t result { NFD::SaveDialog(outPath, kSceneFilter, 1, nullptr, "scene.json") };

            if (result == NFD_OKAY) {
                SaveScene(std::filesystem::path(outPath.get()));
            }
            else if (result == NFD_ERROR) {
                LW_ERROR("ToolsWindow: NFD SaveDialog error: ", NFD::GetError());
            }
        }

        void OpenScene(const std::filesystem::path& path) {
            auto newScene { Leadwort::CreateUnique<Leadwort::Scenes::DefaultScene>() };

            if (!Leadwort::Serialization::SceneSerializer::LoadFromFile(*newScene, path)) {
                LW_ERROR("ToolsWindow: Failed to load scene from: ", path.string());
                return;
            }

            Leadwort::Systems::SceneSystem::Get().LoadScene(std::move(newScene));
            m_EditorContext.OpenedScenePath = path;
            m_EditorContext.ClearSelection();
        }

        void SaveScene(const std::filesystem::path& path) const {
            const auto* currentScene { Leadwort::Systems::SceneSystem::Get().GetCurrentScene() };

            if (!currentScene) {
                LW_ERROR("ToolsWindow: No scene opened to save");
                return;
            }

            if (!Leadwort::Serialization::SceneSerializer::SaveToFile(*currentScene, path)) {
                LW_ERROR("ToolsWindow: Failed to save scene to: ", path.string());
                return;
            }

            m_EditorContext.OpenedScenePath = path;
        }

    private:
        EditorContext& m_EditorContext;
    };

}