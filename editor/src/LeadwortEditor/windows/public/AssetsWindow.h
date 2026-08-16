#pragma once
#include "Leadwort/asset-management/public/AssetDatabase.h"
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

		void DrawFileEntry(const std::filesystem::path& relativePath) {
	    	const auto type { Leadwort::AssetManagement::InferAssetType(relativePath) };

	    	if (type == Leadwort::AssetManagement::AssetType::Model) {
	    		DrawModelEntry(relativePath);
	    		return;
	    	}

	    	ImGui::TreeNodeEx(relativePath.filename().string().c_str(),
				ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);

	    	if (ImGui::IsItemClicked()) {
	    		m_EditorContext.SelectAsset(relativePath, type);
	    	}

	    	if (ImGui::BeginDragDropSource()) {
	    		if (const char* payloadID { GetPayloadIDFor(type) }) {
	    			ImGui::SetDragDropPayload(payloadID, &relativePath, sizeof(std::filesystem::path));
	    			ImGui::Text("%s", relativePath.filename().string().c_str());
	    		}
	    		ImGui::EndDragDropSource();
	    	}
	    }

		static const char* GetPayloadIDFor(const Leadwort::AssetManagement::AssetType type) {
	    	switch (type) {
	    		case Leadwort::AssetManagement::AssetType::Texture:  return "ASSET_TEXTURE";
	    		case Leadwort::AssetManagement::AssetType::Material: return "ASSET_MATERIAL";
				case Leadwort::AssetManagement::AssetType::Shader:   return "ASSET_SHADER";
	    		default: return nullptr;
	    	}
	    }

		void DrawModelEntry(const std::filesystem::path& relativePath) {
	    	using namespace Leadwort;

	    	constexpr ImGuiTreeNodeFlags flags { ImGuiTreeNodeFlags_OpenOnArrow };
	    	const bool open { ImGui::TreeNodeEx(relativePath.filename().string().c_str(), flags) };

	    	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
	    		m_EditorContext.SelectAsset(relativePath, AssetManagement::AssetType::Model);
	    	}

	    	if (open) {
	    		ImGui::Indent(10.0f);

	    		const std::string key = relativePath.string();
	    		auto it { m_LoadedModels.find(key) };
	    		if (it == m_LoadedModels.end()) {
	    			it = m_LoadedModels.emplace(key, AssetManagement::EngineAssets::GetModel(key)).first;
	    		}

				if (const auto& model = it->second) {
	    			for (size_t i = 0; i < model->GetMeshCount(); i++) {
	    				const auto& mesh = model->GetMesh(i);
	    				if (!mesh) {
	    					continue;
	    				}

	    				ImGui::PushID(static_cast<int>(i));
	    				DrawMeshEntry(mesh);
	    				ImGui::PopID();
	    			}
	    		}
	    		else {
	    			ImGui::TextDisabled("Failed to load model");
	    		}

	    		ImGui::Unindent(10.0f);
	    		ImGui::TreePop();
	    	}
	    }

		void DrawMeshEntry(const Leadwort::Shared<Leadwort::Rendering::Bindables::Mesh>& mesh) const {
	    	using namespace Leadwort;

	    	const std::string meshLabel { "Mesh: " + (mesh->GetName().empty() ? "(unnamed)" : mesh->GetName()) };

	    	const bool hasMaterial { mesh->GetSharedMaterial() != nullptr };
	    	const ImGuiTreeNodeFlags meshFlags { hasMaterial
				? ImGuiTreeNodeFlags_OpenOnArrow
				: (ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet)
	    	};

	    	const bool meshOpen { ImGui::TreeNodeEx(meshLabel.c_str(), meshFlags) };

	    	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
	    		m_EditorContext.SelectEmbeddedMesh(mesh);
	    	}

	    	if (!hasMaterial) {
	    		return;
	    	}

	    	if (!meshOpen) {
	    		return;
	    	}

	    	ImGui::Indent(10.0f);

	    	const std::string matLabel { "Material: " + mesh->GetSharedMaterial()->GetName() };

	    	ImGui::TreeNodeEx(matLabel.c_str(),
				ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet
			);

	    	if (ImGui::IsItemClicked()) {
	    		m_EditorContext.SelectEmbeddedMaterial(mesh->GetSharedMaterial());
	    	}

	    	ImGui::Unindent(10.0f);
	    	ImGui::TreePop();
	    }

	private:
	    std::filesystem::path m_SelectedPath{};
		std::unordered_map<std::string, Leadwort::Shared<Leadwort::Core::Model>> m_LoadedModels{};
		EditorContext& m_EditorContext;
	};

}