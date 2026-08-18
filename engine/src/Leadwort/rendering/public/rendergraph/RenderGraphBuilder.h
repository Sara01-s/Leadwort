#pragma once

#include "IPass.h"

#include <unordered_map>
#include <vector>

namespace Leadwort::Rendering::RG {

	enum class AccessFlags {
		Read, Write, ReadWrite
	};

	enum class UsageFlags {
		Sampled, Color, DepthStencil
	};

	class RenderGraphBuilder final {
	public:
		void UseTexture(IPass& pass, RenderTexture& texture, const AccessFlags flags) noexcept {
			if (!m_PassGraph.contains(&pass)) {
				m_PassInsertionOrder.push_back(&pass);
			}

			m_PassGraph[&pass].push_back({ &texture, flags, UsageFlags::Sampled });
		}

		void SetRenderAttachment(IPass& pass, RenderTexture& texture) noexcept {
			if (!m_PassGraph.contains(&pass)) {
				m_PassInsertionOrder.push_back(&pass);
			}

			m_PassGraph[&pass].push_back({ &texture, AccessFlags::Write, UsageFlags::Color });
		}

		void SetDepthAttachment(IPass& pass, RenderTexture& texture) noexcept {
			if (!m_PassGraph.contains(&pass)) {
				m_PassInsertionOrder.push_back(&pass);
			}

			m_PassGraph[&pass].push_back({ &texture, AccessFlags::Write, UsageFlags::DepthStencil });
		}

		[[nodiscard]]
		const auto& GetPassesEntries() const noexcept {
			return m_PassGraph;
		}

		[[nodiscard]]
		auto ResolveResourceWriters() noexcept {
			std::unordered_map<const RenderTexture*, IPass*> writers{};

			for (const auto& [pass, dependencies] : m_PassGraph) {
				for (const auto& [resource, flags, _] : dependencies) {
					if (IsWriteAccess(flags)) {
						writers[resource] = pass;
					}
				}
			}
			
			return writers;
		}

		// Pass A depends on Pass B if:
		//  - A reads a resource that B last wrote, OR
		//  - A writes a resource that B previously wrote (write-after-write ordering (WAW))
		[[nodiscard]]
		auto BuildDependencyGraph() {
			std::unordered_map<IPass*, std::vector<IPass*>> edges{};
			std::unordered_map<const RenderTexture*, IPass*> lastWriter{};

			for (const auto& pass : m_PassGraph | std::views::keys) {
				edges[pass]; // ensure every pass has an entry, even with 0 deps
			}

			// Iterate passes in insertion order, not map order, so WAW edges
			// reflect the order the caller declared them in AddPass().
			for (auto* pass : m_PassInsertionOrder) {
				const auto& dependencies { m_PassGraph.at(pass) };

				// First pass: read dependencies against the *current* writers
				for (const auto& [resource, flags, _] : dependencies) {
					if (IsReadAccess(flags)) {
						auto it = lastWriter.find(resource);
						if (it != lastWriter.end() && it->second != pass) {
							edges[it->second].push_back(pass);
						}
					}
				}

				// Then: write dependency chain off the previous writer (WAW)
				for (const auto& [resource, flags, _] : dependencies) {
					if (IsWriteAccess(flags)) {
						auto it = lastWriter.find(resource);
						if (it != lastWriter.end() && it->second != pass) {
							edges[it->second].push_back(pass);
						}
						lastWriter[resource] = pass;
					}
				}
			}

			return edges;
		}

		auto GetPassGraph() const noexcept {
			return m_PassGraph;
		}

		[[nodiscard]]
		static constexpr bool IsReadAccess(const AccessFlags flags) noexcept {
			return flags == AccessFlags::Read || flags == AccessFlags::ReadWrite;
		}

		[[nodiscard]]
		static constexpr bool IsWriteAccess(const AccessFlags flags) noexcept {
			return flags == AccessFlags::Write || flags == AccessFlags::ReadWrite;
		}

	private:
		struct ResourceDescription {
			RenderTexture* resource { nullptr };
			AccessFlags flags { AccessFlags::Read };
			UsageFlags usage { UsageFlags::Color };
		};

		using PassDependencies = std::vector<ResourceDescription>;
		using PassGraph = std::unordered_map<IPass*, PassDependencies>;

		PassGraph m_PassGraph{};
		std::vector<IPass*> m_PassInsertionOrder{};
	};

}