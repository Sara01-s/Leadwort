#pragma once

#include "LeadwortEditor/core/public/IEditorWindow.h"
#include "imgui.h"

#include <Leadwort/core/public/Game.h>
#include <Leadwort/rendering/public/rendergraph/RenderGraph.h>
#include <Leadwort/rendering/public/rendergraph/RenderTexture.h>

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace Editor::Windows {

	class RenderGraphWindow final : public Core::IEditorWindow {
		using RenderGraph   = Leadwort::Rendering::RG::RenderGraph;
		using RenderTexture = Leadwort::Rendering::RG::RenderTexture;
		using IPass         = Leadwort::Rendering::RG::IPass;
		using AccessFlags   = Leadwort::Rendering::RG::AccessFlags;
		using UsageFlags    = Leadwort::Rendering::RG::UsageFlags;
		using Builder       = Leadwort::Rendering::RG::RenderGraphBuilder;

	public:
		explicit RenderGraphWindow(Leadwort::Core::Game& game)
			: m_Graphs {
				GraphEntry { "Game",  &game.GetGameRenderGraph()  },
				GraphEntry { "Scene", &game.GetSceneRenderGraph() },
			} {}

		std::string_view GetName() const noexcept override { return "Render Graph"; }

		void OnGuiRender() override {
			ImGui::Begin(GetName().data());

			RenderGraph* graph { m_Graphs.empty() ? nullptr : m_Graphs[m_SelectedGraph].Graph };

			if (graph == nullptr) {
				ImGui::TextDisabled("No render graph available.");
				ImGui::End();
				return;
			}

			DrawToolbar(*graph);
			ImGui::Separator();

			if (!graph->IsCompiled()) {
				ImGui::TextColored(kWarningColor, "Graph is not compiled: a cycle was detected, or Compile() was never called.");
				ImGui::End();
				return;
			}

			RebuildMatrix(*graph);

			if (m_SelectedPass >= static_cast<int>(m_Passes.size())) {
				m_SelectedPass = -1;
			}

			if (m_Passes.empty()) {
				ImGui::TextDisabled("This graph has no passes.");
				ImGui::End();
				return;
			}

			DrawMatrix();
			DrawDetailsPanel();

			ImGui::End();
		}

	private:
		struct GraphEntry {
			const char* Label { nullptr };
			RenderGraph* Graph { nullptr };
		};

		struct ResourceRow {
			const RenderTexture* Resource { nullptr };
			int FirstPass { 0 };
			int LastPass { 0 };
		};

		struct Cell {
			bool Used { false };
			bool Read { false };
			bool Write { false };
			UsageFlags Usage { UsageFlags::Color };
		};

		[[nodiscard]] int FindRow(const RenderTexture* resource) const noexcept {
			for (std::size_t i = 0; i < m_Rows.size(); ++i) {
				if (m_Rows[i].Resource == resource) {
					return static_cast<int>(i);
				}
			}

			return -1;
		}

		[[nodiscard]] const Cell& GetCell(const int row, const int col) const noexcept {
			return m_Cells[static_cast<std::size_t>(row) * m_Passes.size() + static_cast<std::size_t>(col)];
		}

		void RebuildMatrix(const RenderGraph& graph) {
			m_Passes.clear();
			m_Rows.clear();
			m_Cells.clear();

			for (const auto& pass : graph.GetPasses()) {
				m_Passes.push_back(pass.get());
			}

			const auto& entries { graph.GetBuilder().GetPassesEntries() };

			for (std::size_t passIndex = 0; passIndex < m_Passes.size(); ++passIndex) {
				const auto it { entries.find(m_Passes[passIndex]) };
				if (it == entries.end()) {
					continue;
				}

				for (const auto& dependency : it->second) {
					if (FindRow(dependency.resource) < 0) {
						m_Rows.push_back(ResourceRow {
							dependency.resource,
							static_cast<int>(passIndex),
							static_cast<int>(passIndex)
						});
					}
				}
			}

			m_Cells.assign(m_Rows.size() * m_Passes.size(), Cell{});

			for (std::size_t passIndex = 0; passIndex < m_Passes.size(); ++passIndex) {
				const auto it { entries.find(m_Passes[passIndex]) };
				if (it == entries.end()) {
					continue;
				}

				for (const auto& [resource, flags, usage] : it->second) {
					const int row { FindRow(resource) };
					if (row < 0) {
						continue;
					}

					Cell& cell { m_Cells[static_cast<std::size_t>(row) * m_Passes.size() + passIndex] };
					cell.Used = true;
					cell.Read = cell.Read || Builder::IsReadAccess(flags);
					cell.Write = cell.Write || Builder::IsWriteAccess(flags);
					cell.Usage = usage;

					m_Rows[row].LastPass = static_cast<int>(passIndex);
				}
			}
		}

		void DrawToolbar(RenderGraph& graph) {
			ImGui::SetNextItemWidth(140.0f);

			if (ImGui::BeginCombo("##graph", m_Graphs[m_SelectedGraph].Label)) {
				for (int i = 0; i < static_cast<int>(m_Graphs.size()); ++i) {
					if (ImGui::Selectable(m_Graphs[i].Label, i == m_SelectedGraph)) {
						m_SelectedGraph = i;
						m_SelectedPass = -1;
					}
				}

				ImGui::EndCombo();
			}

			ImGui::SameLine();
			if (ImGui::Button("Enable All")) {
				SetAllPasses(graph, true);
			}

			ImGui::SameLine();
			if (ImGui::Button("Disable All")) {
				SetAllPasses(graph, false);
			}

			ImGui::SameLine();
			ImGui::TextDisabled("|");
			ImGui::SameLine();

			DrawLegendSwatch(ColorRead, "Read");
			DrawLegendSwatch(ColorWrite, "Write");
			DrawLegendSwatch(ColorDepth, "Depth / Stencil");

			ImGui::NewLine();
		}

		static void SetAllPasses(const RenderGraph& graph, const bool enabled) {
			for (const auto& pass : graph.GetPasses()) {
				pass->SetEnabled(enabled);
			}
		}

		static void DrawLegendSwatch(const ImU32 color, const char* label) {
			ImDrawList* drawList { ImGui::GetWindowDrawList() };

			const ImVec2 pos { ImGui::GetCursorScreenPos() };
			const float lineHeight { ImGui::GetTextLineHeight() };
			const float size { lineHeight * 0.7f };
			const float offset { (lineHeight - size) * 0.5f };

			drawList->AddRectFilled(
				ImVec2(pos.x, pos.y + offset),
				ImVec2(pos.x + size, pos.y + offset + size),
				color, 2.0f
			);

			ImGui::Dummy(ImVec2(size, lineHeight));
			ImGui::SameLine(0.0f, 5.0f);
			ImGui::TextUnformatted(label);
			ImGui::SameLine(0.0f, 14.0f);
		}

		void DrawMatrix() {
			float longestNameWidth { 0.0f };
			for (const IPass* pass : m_Passes) {
				const std::string_view name { pass->GetName() };
				longestNameWidth = std::max(longestNameWidth, ImGui::CalcTextSize(name.data(), name.data() + name.size()).x);
			}

			const float headerHeight { longestNameWidth + ImGui::GetFrameHeight() + 18.0f };
			const float rowHeight { ImGui::GetTextLineHeight() + 12.0f };

			const float availableHeight { ImGui::GetContentRegionAvail().y };
			const float tableHeight { std::max(availableHeight - DetailsHeight, headerHeight + rowHeight * 2.0f) };

			constexpr ImGuiTableFlags tableFlags {
				ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoPadInnerX
				| ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
			};

			if (!ImGui::BeginTable("##rendergraph_matrix", 1 + static_cast<int>(m_Passes.size()), tableFlags, ImVec2(0.0f, tableHeight))) {
				return;
			}

			ImGui::TableSetupColumn("Resource", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, ResourceColumnWidth);

			for (const IPass* pass : m_Passes) {
				ImGui::TableSetupColumn(pass->GetName().data(), ImGuiTableColumnFlags_WidthFixed, PassColumnWidth);
			}

			ImGui::TableSetupScrollFreeze(1, 1);

			m_NextHoveredPass = -1;

			// Header row: enable toggle + vertical pass name, in execution order.
			ImGui::TableNextRow(ImGuiTableRowFlags_Headers, headerHeight);

			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::TextDisabled("Resources (%d)", static_cast<int>(m_Rows.size()));

			for (int col = 0; col < static_cast<int>(m_Passes.size()); ++col) {
				ImGui::TableSetColumnIndex(col + 1);
				DrawPassHeader(col, headerHeight);
			}

			for (int row = 0; row < static_cast<int>(m_Rows.size()); ++row) {
				ImGui::TableNextRow(0, rowHeight);

				ImGui::TableSetColumnIndex(0);
				DrawResourceLabel(m_Rows[row]);

				for (int col = 0; col < static_cast<int>(m_Passes.size()); ++col) {
					ImGui::TableSetColumnIndex(col + 1);
					DrawUsageCell(row, col, rowHeight);
				}
			}

			ImGui::EndTable();

			// Row/column highlighting trails the cursor by one frame: cell backgrounds
			// are submitted before we know what the pointer ended up over.
			m_HoveredPass = m_NextHoveredPass;
		}

		void DrawPassHeader(const int passIndex, const float headerHeight) {
			IPass* pass { m_Passes[passIndex] };

			const bool isSelected { passIndex == m_SelectedPass };

			if (isSelected) {
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, SelectedColumnColor);
			}
			else if (passIndex == m_HoveredPass) {
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, HoveredColumnColor);
			}

			const ImVec2 cellMin { ImGui::GetCursorScreenPos() };
			const float columnWidth { std::max(ImGui::GetContentRegionAvail().x, 1.0f) };
			const float frameHeight { ImGui::GetFrameHeight() };

			ImDrawList* drawList { ImGui::GetWindowDrawList() };

			ImGui::PushID(passIndex);

			ImGui::SetCursorScreenPos(ImVec2(cellMin.x + (columnWidth - frameHeight) * 0.5f, cellMin.y + 3.0f));

			bool enabled { pass->IsEnabled() };
			if (ImGui::Checkbox("##enabled", &enabled)) {
				pass->SetEnabled(enabled);
			}

			if (ImGui::IsItemHovered()) {
				m_NextHoveredPass = passIndex;
				const std::string_view name { pass->GetName() };
				ImGui::SetTooltip("%.*s\n%s", static_cast<int>(name.size()), name.data(), enabled ? "Enabled" : "Disabled");
			}

			// Clickable name strip below the toggle.
			const float nameTop { cellMin.y + frameHeight + 5.0f };
			const float nameHeight { std::max(cellMin.y + headerHeight - nameTop - 6.0f, 1.0f) };

			ImGui::SetCursorScreenPos(ImVec2(cellMin.x, nameTop));
			ImGui::InvisibleButton("##select", ImVec2(columnWidth, nameHeight));

			if (ImGui::IsItemHovered()) {
				m_NextHoveredPass = passIndex;
			}

			if (ImGui::IsItemClicked()) {
				m_SelectedPass = isSelected ? -1 : passIndex;
			}

			ImU32 textColor { PassNameColor };
			if (!enabled) {
				textColor = DisabledTextColor;
			}
			else if (isSelected) {
				textColor = SelectedTextColor;
			}

			const ImGuiViewport* viewport { ImGui::GetMainViewport() };
			drawList->PushClipRect(
				ImVec2(viewport->Pos.x, cellMin.y),
				ImVec2(viewport->Pos.x + viewport->Size.x, cellMin.y + headerHeight),
				false
			);

			AddTextRotated(
				drawList,
				ImVec2(cellMin.x + (columnWidth - ImGui::GetTextLineHeight()) * 0.5f, cellMin.y + headerHeight - 8.0f),
				textColor,
				pass->GetName()
			);

			drawList->PopClipRect();

			ImGui::PopID();
		}

		static void DrawResourceLabel(const ResourceRow& row) {
			const RenderTexture& texture { *row.Resource };

			const ImVec2 pos { ImGui::GetCursorScreenPos() };
			const float lineHeight { ImGui::GetTextLineHeight() };
			const float swatchSize { lineHeight * 0.75f };

			ImGui::GetWindowDrawList()->AddRectFilled(
				ImVec2(pos.x + 5.0f, pos.y + (lineHeight - swatchSize) * 0.5f),
				ImVec2(pos.x + 5.0f + swatchSize, pos.y + (lineHeight + swatchSize) * 0.5f),
				ResourceColor(texture), 2.0f
			);

			ImGui::SetCursorScreenPos(ImVec2(pos.x + swatchSize + 12.0f, pos.y));

			const char* name { texture.GetDebugName().empty() ? "<unnamed>" : texture.GetDebugName().c_str() };
			ImGui::TextUnformatted(name);

			const bool nameHovered { ImGui::IsItemHovered() };

			const std::string_view format { RenderTexture::GetFormatName(texture.GetFormat()) };
			ImGui::SameLine(0.0f, 8.0f);
			ImGui::TextDisabled("%.*s", static_cast<int>(format.size()), format.data());

			if (nameHovered || ImGui::IsItemHovered()) {
				ImGui::SetTooltip(
					"%s\nFormat: %.*s\nSize: %d x %d\nGPU ID: %u\nLifetime: pass %d -> %d",
					name,
					static_cast<int>(format.size()), format.data(),
					texture.GetWidth(), texture.GetHeight(),
					texture.GetGpuID(),
					row.FirstPass + 1, row.LastPass + 1
				);
			}
		}

		void DrawUsageCell(const int row, const int col, const float rowHeight) {
			const ResourceRow& resourceRow { m_Rows[row] };
			const Cell& cell { GetCell(row, col) };
			const IPass* pass { m_Passes[col] };
			const bool passEnabled { pass->IsEnabled() };

			if (col == m_SelectedPass) {
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, SelectedColumnColor);
			}
			else if (col == m_HoveredPass) {
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, HoveredColumnColor);
			}

			const ImGuiStyle& style { ImGui::GetStyle() };
			const ImVec2 cellMin { ImGui::GetCursorScreenPos() };
			const float columnWidth { std::max(ImGui::GetContentRegionAvail().x, 1.0f) };
			const float centerX { cellMin.x + columnWidth * 0.5f };
			const float centerY { cellMin.y + rowHeight * 0.5f - style.CellPadding.y };

			ImDrawList* drawList { ImGui::GetWindowDrawList() };

			// Lifetime line, drawn per cell so it reads as one continuous run.
			if (col >= resourceRow.FirstPass && col <= resourceRow.LastPass) {
				const float lineStart { col == resourceRow.FirstPass ? centerX : cellMin.x - style.CellPadding.x };
				const float lineEnd { col == resourceRow.LastPass ? centerX : cellMin.x + columnWidth + style.CellPadding.x };

				drawList->AddLine(ImVec2(lineStart, centerY), ImVec2(lineEnd, centerY), ColorLifetime, 2.0f);
			}

			if (!cell.Used) {
				return;
			}

			const float blockWidth { std::min(columnWidth - 8.0f, 18.0f) };
			const float blockHeight { std::min(rowHeight - 8.0f, 16.0f) };

			const ImVec2 blockMin { centerX - blockWidth * 0.5f, centerY - blockHeight * 0.5f };
			const ImVec2 blockMax { centerX + blockWidth * 0.5f, centerY + blockHeight * 0.5f };

			const int alpha { passEnabled ? 255 : 70 };

			if (cell.Read && cell.Write) {
				drawList->AddRectFilled(blockMin, ImVec2(centerX, blockMax.y), WithAlpha(ColorRead, alpha), 3.0f, ImDrawFlags_RoundCornersLeft);
				drawList->AddRectFilled(ImVec2(centerX, blockMin.y), blockMax, WithAlpha(ColorWrite, alpha), 3.0f, ImDrawFlags_RoundCornersRight);
			}
			else {
				drawList->AddRectFilled(blockMin, blockMax, WithAlpha(cell.Read ? ColorRead : ColorWrite, alpha), 3.0f);
			}

			// Depth/stencil attachments get an outline so they read apart from color ones.
			if (cell.Usage == UsageFlags::DepthStencil) {
				drawList->AddRect(blockMin, blockMax, WithAlpha(ColorDepth, passEnabled ? 220 : 70), 3.0f, 0, 1.6f);
			}

			ImGui::PushID(row);
			ImGui::PushID(col);

			ImGui::SetCursorScreenPos(cellMin);
			ImGui::InvisibleButton("##cell", ImVec2(columnWidth, std::max(rowHeight - style.CellPadding.y * 2.0f, 1.0f)));

			if (ImGui::IsItemHovered()) {
				m_NextHoveredPass = col;

				const std::string_view passName { pass->GetName() };
				const char* resourceName { resourceRow.Resource->GetDebugName().empty() ? "<unnamed>" : resourceRow.Resource->GetDebugName().c_str() };

				ImGui::SetTooltip(
					"%.*s\n%s\nAccess: %s\nUsage: %s",
					static_cast<int>(passName.size()), passName.data(),
					resourceName,
					AccessName(cell),
					UsageName(cell.Usage)
				);
			}

			if (ImGui::IsItemClicked()) {
				m_SelectedPass = col;
			}

			ImGui::PopID();
			ImGui::PopID();
		}

		void DrawDetailsPanel() const {
			ImGui::Separator();

			if (m_SelectedPass < 0) {
				ImGui::TextDisabled("Select a pass column to inspect it.");
				return;
			}

			IPass* pass { m_Passes[m_SelectedPass] };
			const std::string_view name { pass->GetName() };

			ImGui::Text("%.*s", static_cast<int>(name.size()), name.data());
			ImGui::SameLine(0.0f, 16.0f);

			bool enabled { pass->IsEnabled() };
			if (ImGui::Checkbox("Enabled", &enabled)) {
				pass->SetEnabled(enabled);
			}

			ImGui::TextDisabled(
				"Pass ID %u    Execution order %d / %d",
				pass->GetID(), m_SelectedPass + 1, static_cast<int>(m_Passes.size())
			);

			if (const auto* frameBuffer { pass->GetFrameBuffer() }) {
				ImGui::TextDisabled(
					"Framebuffer FBO %u    %d x %d",
					frameBuffer->GetFboID(), frameBuffer->GetWidth(), frameBuffer->GetHeight()
				);
			}
			else {
				ImGui::TextDisabled("No framebuffer (pass declares no write attachments)");
			}

			constexpr ImGuiTableFlags detailsFlags {
				ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp
				| ImGuiTableFlags_ScrollY
			};

			const float detailsTableHeight { ImGui::GetContentRegionAvail().y };
			if (detailsTableHeight < ImGui::GetFrameHeight()) {
				return;
			}

			if (!ImGui::BeginTable("##pass_resources", 4, detailsFlags, ImVec2(0.0f, detailsTableHeight))) {
				return;
			}

			ImGui::TableSetupColumn("Resource");
			ImGui::TableSetupColumn("Access");
			ImGui::TableSetupColumn("Usage");
			ImGui::TableSetupColumn("Format");
			ImGui::TableHeadersRow();

			for (int row = 0; row < static_cast<int>(m_Rows.size()); ++row) {
				const Cell& cell { GetCell(row, m_SelectedPass) };
				if (!cell.Used) {
					continue;
				}

				const RenderTexture& texture { *m_Rows[row].Resource };
				const std::string_view format { RenderTexture::GetFormatName(texture.GetFormat()) };

				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImGui::TextUnformatted(texture.GetDebugName().empty() ? "<unnamed>" : texture.GetDebugName().c_str());

				ImGui::TableNextColumn();
				ImGui::TextUnformatted(AccessName(cell));

				ImGui::TableNextColumn();
				ImGui::TextUnformatted(UsageName(cell.Usage));

				ImGui::TableNextColumn();
				ImGui::Text("%.*s  (%d x %d)", static_cast<int>(format.size()), format.data(), texture.GetWidth(), texture.GetHeight());
			}

			ImGui::EndTable();
		}

		static void AddTextRotated(ImDrawList* drawList, const ImVec2& bottomLeft, const ImU32 color, const std::string_view text) {
			const int firstVertex { drawList->VtxBuffer.Size };

			drawList->AddText(bottomLeft, color, text.data(), text.data() + text.size());

			for (int i = firstVertex; i < drawList->VtxBuffer.Size; ++i) {
				ImDrawVert& vertex { drawList->VtxBuffer[i] };

				const float dx { vertex.pos.x - bottomLeft.x };
				const float dy { vertex.pos.y - bottomLeft.y };

				vertex.pos.x = bottomLeft.x + dy;
				vertex.pos.y = bottomLeft.y - dx;
			}
		}

		static constexpr ImU32 WithAlpha(const ImU32 color, const int alpha) noexcept {
			return (color & ~IM_COL32_A_MASK) | (static_cast<ImU32>(alpha) << IM_COL32_A_SHIFT);
		}

		static ImU32 ResourceColor(const RenderTexture& texture) noexcept {
			if (texture.IsShadowSampleable()) {
				return IM_COL32(232, 205, 90, 255);
			}

			return texture.IsDepth() ? ColorDepth : IM_COL32(92, 208, 168, 255);
		}

		static const char* AccessName(const Cell& cell) noexcept {
			if (cell.Read && cell.Write) {
				return "Read / Write";
			}

			return cell.Read ? "Read" : "Write";
		}

		static const char* UsageName(const UsageFlags usage) noexcept {
			switch (usage) {
				case UsageFlags::Sampled:      return "Sampled";
				case UsageFlags::Color:        return "Color attachment";
				case UsageFlags::DepthStencil: return "Depth / stencil attachment";
			}

			return "Unknown";
		}

	private:
		static constexpr float PassColumnWidth { 30.0f };
		static constexpr float ResourceColumnWidth { 250.0f };
		static constexpr float DetailsHeight { 210.0f };

		static constexpr ImU32 ColorRead { IM_COL32(86, 156, 240, 255) };
		static constexpr ImU32 ColorWrite { IM_COL32(240, 158, 62, 255) };
		static constexpr ImU32 ColorDepth { IM_COL32(176, 128, 232, 255) };
		static constexpr ImU32 ColorLifetime { IM_COL32(255, 255, 255, 46) };

		static constexpr ImU32 PassNameColor { IM_COL32(215, 215, 215, 255) };
		static constexpr ImU32 SelectedTextColor { IM_COL32(255, 255, 255, 255) };
		static constexpr ImU32 DisabledTextColor { IM_COL32(140, 140, 140, 255) };

		static constexpr ImU32 SelectedColumnColor { IM_COL32(80, 150, 230, 48) };
		static constexpr ImU32 HoveredColumnColor { IM_COL32(255, 255, 255, 14) };

		static constexpr ImVec4 kWarningColor { 1.0f, 0.45f, 0.35f, 1.0f };

	private:
		std::vector<GraphEntry> m_Graphs{};
		int m_SelectedGraph { 0 };

		std::vector<IPass*> m_Passes{};
		std::vector<ResourceRow> m_Rows{};
		std::vector<Cell> m_Cells{};

		int m_SelectedPass { -1 };
		int m_HoveredPass { -1 };
		int m_NextHoveredPass { -1 };
	};

} // namespace Editor::Windows
