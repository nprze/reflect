#pragma once
#include "fg_resources.h"
#include "context.h"

namespace rfct {
	using CommandList = std::vector<std::string>;

	enum class RfctFGResourceState {
		Undefined,
		RenderTarget,
		ShaderRead,
		Present
	};

	enum class RfctFGFormat {
		RGBA8,
		D32F
	};

	struct RfctFGResourceHandle {
		uint32_t resourceIndex = UINT32_MAX;
		bool IsValid() const { return resourceIndex != UINT32_MAX; }
	};

	struct RfctFGPassHandle {
		uint32_t passIndex = UINT32_MAX;
		bool IsValid() const { return passIndex != UINT32_MAX; }
	};

	struct RfctFGMemBlockHandle {
		uint32_t blockIndex = UINT32_MAX;
		bool IsValid() const { return blockIndex != UINT32_MAX; }
	};

	struct RfctFGResourceLifetime {
		RfctFGPassHandle firstUsePass; // sorted pass index
		RfctFGPassHandle lastUsePass = { 0 }; // last sorted pass
	};

	struct RfctFGBarrier {
		uint32_t resourceIndex;
		RfctFGResourceState oldState;
		RfctFGResourceState newState;
	};

	struct RfctFGResourceDesc {
		uint32_t width = 0;
		uint32_t height = 0;
		RfctFGFormat format = RfctFGFormat::RGBA8;
		std::string name;
	};

	struct RfctFGResourceVersion {
		RfctFGPassHandle writerPass;
		std::vector<RfctFGPassHandle> readerPasses;
		bool HasWriter() const { return writerPass.IsValid(); }
	};

	struct RfctFGResourceEntry {
		RfctFGResourceDesc desc;
		std::vector<RfctFGResourceVersion> versions;
		RfctFGResourceState currentState = RfctFGResourceState::Undefined;
	};

	struct RfctFGRenderPass {
		std::string name;
		std::function<void(RfctFGPassHandle)> Setup;
		std::function<void(CommandList*)> Execute;
		std::vector<RfctFGResourceHandle> reads;
		std::vector<RfctFGResourceHandle> writes;
		std::vector<RfctFGPassHandle> dependsOnPasses;
		std::vector<RfctFGPassHandle> successorPasses;
		uint32_t inDegree = 0;
		bool used = false; // for culling
	};

	struct RfctFGCompiledPlan {
		std::vector<RfctFGPassHandle> sortedPasses;
		std::vector<RfctFGMemBlockHandle> memBlockMapping;
		std::vector<std::vector<RfctFGBarrier>> barriers;
	};

	class RfctFrameGraph {
	public:
		RfctFGPassHandle AddPass(const std::string& name, std::function<void(RfctFGPassHandle)>&& setup, std::function<void(CommandList*)>&& exec);
		RfctFGResourceHandle ImportResource(const RfctFGResourceDesc& desc, RfctFGResourceState initialState = RfctFGResourceState::Undefined);
		void Read(RfctFGPassHandle passIdx, RfctFGResourceHandle handle);
		void Write(RfctFGPassHandle passIdx, RfctFGResourceHandle handle);
		RfctFGCompiledPlan Compile();
		void Execute(CommandList* cmdList);
		void Reset();
		void ForgetAllResources();
		RfctFrameGraphResources& GetResources() { return m_resources; };
	private:
		// building
		void BuildEdges();
		std::vector<RfctFGPassHandle> TopoSort();
		void Cull(const std::vector<RfctFGPassHandle>& sortedPasses);
		RfctFGResourceState StateForUsage(RfctFGResourceHandle h, bool isWrite);
		std::vector<std::vector<RfctFGBarrier>> ComputeBarriers(const std::vector<RfctFGPassHandle>& sortedPasses, const std::vector<RfctFGMemBlockHandle>& blockMapping);
		// execution
		void Execute(const RfctFGCompiledPlan& plan, CommandList* cmdList);
		void ApplyBarriers(const std::vector<RfctFGBarrier>& barriers, CommandList* cmdList);
		void ApplyBarrier(const RfctFGBarrier& barrier, CommandList* cmdList);
		// resource aliasing
		std::vector<RfctFGResourceLifetime> ScanLifetimes(const std::vector<RfctFGPassHandle>& sorted);
		std::vector<RfctFGMemBlockHandle> AliasResources(const std::vector<RfctFGResourceLifetime>& lifetimes);
	private:
		std::vector<RfctFGRenderPass> m_passes;
		std::vector<RfctFGResourceEntry> m_entries;
		RfctFrameGraphResources m_resources;
	};
};