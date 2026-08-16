#pragma once

#include <vector>
#include <unordered_map>
#include <queue>

#include "FrameBuffer.h"
#include "IPass.h"
#include "Leadwort/core/public/Core.h"
#include "Leadwort/utils/public/Logger.h"
#include "RenderGraphBuilder.h"

#include <ranges>

namespace Leadwort::Rendering::RG {

    class RenderGraph final {
    public:
    	explicit RenderGraph(const std::string_view name = "Render Graph") : m_Name(name) {
    		m_RenderGraphBuilder = std::make_unique<RenderGraphBuilder>();
    	}

    	template<typename TPass, typename... Args>
		requires(std::is_base_of_v<IPass, TPass>)
		void AddPass(Args&&... args) {
    		auto pass { CreateUnique<TPass>(std::forward<Args>(args)...) };
    		pass->RecordToRenderGraph(*m_RenderGraphBuilder);

    		m_Passes.emplace_back(std::move(pass));
    	}

    	void Compile() {
    		// Print();
    		m_IsCompiled = false;
    		m_FrameBuffers.clear();

    		TopologicalSort();

    		if (!m_IsCompiled) {
    			return;
    		}

    		BuildFrameBuffers();
    		// PrintSortedPasses();
    	}

    	void Execute(Components::Camera& camera, RenderQueues& queues, Components::MeshRenderer* highlightedMeshRenderer) const {
    		if (!m_IsCompiled) {
    			LW_ERROR("RenderGraph: Cannot Execute, graph is not compiled or has errors");
    			return;
    		}

    		const RenderContext renderContext {
    			&camera,
				&queues,
				highlightedMeshRenderer
			};

    		for (const auto& pass : m_Passes) {
    			if (const auto* frameBuffer = pass->GetFrameBuffer()) {
    				frameBuffer->Bind();
    			}

    			pass->Execute(renderContext);
    		}

    		FrameBuffer::Unbind();
    	}

    	void Print() const {
    		LW_LOG("Render Graph: ", m_Name);

    		for (const auto& [pass, dependencies] : m_RenderGraphBuilder->GetPassesEntries()) {
    			LW_LOG("Pass ID: ", pass->GetID(), " Name: ", pass->GetName(), " Dependencies: ", dependencies.size());

    			for (const auto& [resource, flags, _] : dependencies) {
					std::string accessStr{};
    				switch (flags) {
    					case AccessFlags::Read:
    						accessStr = "Read";
    						break;
    					case AccessFlags::Write:
    						accessStr = "Write";
    						break;
    					case AccessFlags::ReadWrite:
    						accessStr = "Read / Write";
    						break;
    					default:
    						LW_ERROR("Invalid AccessFlags");
    						break;
    				}

    				LW_LOG(" - Resource GPU ID : ", resource->GetGpuID(), " | Access = ", accessStr);
    			}
    		}
    	}

    	void PrintSortedPasses() const {
    		LW_LOG("Render Graph: ", m_Name, " - Compiled Order");

    		if (!m_IsCompiled) {
				LW_WARN("Invalid Render Graph: Cycle detected or Compile() not called");
    			return;
    		}

    		for (std::size_t i = 0; i < m_Passes.size(); ++i) {
    			const IPass& pass { *m_Passes[i] };
    			LW_LOG( "  [", i, "] Pass ID: ", pass.GetID(), " | Name: ", pass.GetName());
    		}
    	}

    private:
    	// Kahn's Algorithm
    	void TopologicalSort() noexcept {
    		auto edges { m_RenderGraphBuilder->BuildDependencyGraph() };

    		std::unordered_map<IPass*, int> inDegree{};
    		for (const auto& pass: edges | std::views::keys) {
    			inDegree[pass] = 0;
    		}

    		for (const auto& tos: edges | std::views::values) {
    			for (auto* to : tos) {
    				inDegree[to]++;
    			}
    		}

    		std::queue<IPass*> ready{};
    		for (const auto [pass, degree] : inDegree) {
    			if (degree == 0) {
    				ready.push(pass);
    			}
    		}

    		std::vector<IPass*> sortedOrder{};
    		while (!ready.empty()) {
    			IPass* currentPass { ready.front() };
    			ready.pop();
    			sortedOrder.emplace_back(currentPass);

    			for (auto* nextPass : edges[currentPass]) {
    				if (--inDegree[nextPass] == 0) {
    					ready.push(nextPass);
    				}
    			}
    		}

    		m_IsCompiled = (sortedOrder.size() == inDegree.size());

    		if (!m_IsCompiled) {
    			LW_ERROR("Render Graph Error: Cycle detected! Cannot Compile");
    			return;
    		}

    		std::unordered_map<IPass*, Unique<IPass>> byPtr{};
    		for (auto& pass : m_Passes) {
    			byPtr[pass.get()] = std::move(pass);
    		}

    		m_Passes.clear();
    		for (IPass* pass : sortedOrder) {
    			m_Passes.emplace_back(std::move(byPtr[pass]));
    		}
    	}

    	void BuildFrameBuffers() {
    		const auto& entries { m_RenderGraphBuilder->GetPassesEntries() };

    		for (const auto& pass : m_Passes) {
    			auto it = entries.find(pass.get());
    			if (it == entries.end()) {
    				continue;
    			}

    			std::vector<RenderTexture*> colorAttachments{};
    			std::optional<RenderTexture*> depthAttachment{};

    			for (const auto& [resource, flags, usage] : it->second) {
    				if (!RenderGraphBuilder::IsWriteAccess(flags)) {
    					continue;
    				}

    				if (usage == UsageFlags::DepthStencil) {
    					depthAttachment = resource;
    				}
    				else if (usage == UsageFlags::Color) {
    					colorAttachments.push_back(resource);
    				}
    			}

    			if (colorAttachments.empty() && !depthAttachment) {
    				continue;
    			}

    			auto frameBuffer { CreateUnique<FrameBuffer>(colorAttachments, depthAttachment) };
    			pass->SetFrameBuffer(frameBuffer.get());
    			m_FrameBuffers.push_back(std::move(frameBuffer));
    		}
    	}

	private:
        std::string m_Name{};
		Unique<RenderGraphBuilder> m_RenderGraphBuilder{};
		std::vector<Unique<IPass>> m_Passes{};
    	std::vector<Unique<FrameBuffer>> m_FrameBuffers{};

		bool m_IsCompiled { false };
    };

}