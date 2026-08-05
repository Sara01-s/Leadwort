#pragma once
#include "Leadwort/asset-management/public/AssetManager.h"
#include "Leadwort/asset-management/public/AssetTypes.h"
#include "LeadwortEditor/core/public/IEditorWindow.h"
#include "LeadwortEditor/data/EditorContext.h"
#include "imgui.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace Editor::Windows {

class AssetsWindow final : public Core::IEditorWindow {
public:
	explicit AssetsWindow(EditorContext& m_editor_context) : m_EditorContext(m_editor_context) {}

	std::string_view GetName() const noexcept override { return "Assets"; }

    void OnGuiRender() override {
        if (ImGui::Begin(GetName().data())) {
            if (ImGui::BeginTabBar("AssetsTabBar")) {
                if (ImGui::BeginTabItem("Game")) {
                    DrawAssetTree(Leadwort::AssetManagement::GameAssets::GetRootPath());
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Engine")) {
                    DrawAssetTree(Leadwort::AssetManagement::EngineAssets::GetRootPath());
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
			m_EditorContext.ClearSelection();
		}

        ImGui::End();
    }

private:
    struct FolderNode {
        std::map<std::string, FolderNode> subfolders{};
        std::vector<std::filesystem::path> files{};
    };

    struct CachedTree {
        FolderNode root{};
        std::filesystem::file_time_type lastScan{};
    };

    std::unordered_map<std::string, CachedTree> m_TreeCache;

    void DrawAssetTree(const std::string& rootPath) {
        auto& [root, lastScan] { m_TreeCache[rootPath] };

        if (ImGui::Button("Refresh")) {
            root = BuildTree(rootPath);
        }

        if (root.subfolders.empty() && root.files.empty()) {
            root = BuildTree(rootPath);
        }

        ImGui::SameLine();
        ImGui::TextDisabled("%s", rootPath.c_str());
        ImGui::Separator();

        DrawNode(root, "");
    }

	static FolderNode BuildTree(const std::string& rootPath) {
        FolderNode root{};
        std::error_code ec{};

        if (!std::filesystem::exists(rootPath, ec) || ec) {
            return root;
        }

        for (const auto& entry : std::filesystem::recursive_directory_iterator(rootPath, std::filesystem::directory_options::skip_permission_denied)) {
            if (entry.is_directory()) {
                continue;
            }

            const auto relative { std::filesystem::relative(entry.path(), rootPath) };
            FolderNode* current { &root };

            for (auto it = relative.begin(); it != std::prev(relative.end()); ++it) {
                current = &current->subfolders[it->string()];
            }

            current->files.push_back(relative);
        }

        return root;
    }

    void DrawNode(const FolderNode& node, const std::string& label) {
		constexpr ImGuiTreeNodeFlags flags { ImGuiTreeNodeFlags_OpenOnArrow };

        const bool isRoot { label.empty() };

        if (isRoot || ImGui::TreeNodeEx(label.c_str(), flags)) {
			ImGui::Indent(10.0f);

            for (const auto& [name, child] : node.subfolders) {
                ImGui::PushID(name.c_str());
                DrawNode(child, name);
                ImGui::PopID();
            }

            for (const auto& filePath : node.files) {
                ImGui::PushID(filePath.string().c_str());
                DrawFileEntry(filePath);
                ImGui::PopID();
            }

            if (!isRoot) {
                ImGui::TreePop();
            }

        	ImGui::Unindent(10.0f);
        }
    }

	void DrawFileEntry(const std::filesystem::path& relativePath) const {
    	ImGui::TreeNodeEx(relativePath.filename().string().c_str(),
			ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);

    	if (ImGui::IsItemClicked()) {
    		const auto type = Leadwort::AssetManagement::InferAssetType(relativePath);
    		m_EditorContext.SelectAsset(relativePath, type);
    	}
    }

private:
    std::filesystem::path m_SelectedPath{};
	EditorContext& m_EditorContext;
};

}