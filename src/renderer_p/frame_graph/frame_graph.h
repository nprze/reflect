#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace rfct {
	using CommandList = std::string;

	static constexpr uint32_t kPlacementAlignment = 65536; // 64 KB

	enum class Format {
		RGBA8,
		D32F
	};

	enum class ResourceState {
		Undefined,
		RenderTarget,
		ShaderRead,
		Present
	};

	struct ResourceDesc {
		uint32_t width = 0;
		uint32_t height = 0;
		Format format = Format::RGBA8;
		std::string name;
	};

	struct ResourceHandle {
		uint32_t resourceIndex = UINT32_MAX;
		bool IsValid() const { return resourceIndex != UINT32_MAX; }
	};

	struct ResourceVersion {
		uint32_t writerPassIndex = UINT32_MAX;
		std::vector<uint32_t> readerPassIndices;
		bool HasWriter() const { return writerPassIndex != UINT32_MAX; }
	};

	struct ResourceEntry {
		ResourceDesc desc;
		std::vector<ResourceVersion> versions;
		ResourceState currentState = ResourceState::Undefined;
		bool imported = false; // e.g swapchain is imported
	};

	struct PhysicalBlock {
		uint32_t sizeBytes = 0;
		uint32_t availAfterPass = 0; // sorted
	};

	struct Lifetime {
		uint32_t firstUsePass = UINT32_MAX; // sorted pass index
		uint32_t lastUsePass = 0; // last sorted pass
		bool isTransient = true; // false for externally owned resources
	};

	struct Barrier {
		uint32_t resourceIndex;
		ResourceState oldState;
		ResourceState newState;
		bool isAliasing = false;
		uint32_t aliasBeforeResource = UINT32_MAX;
	};

	struct RenderPass {
		std::string name;
		std::function<void(uint32_t)> Setup;
		std::function<void(CommandList*)> Execute;
		std::vector<ResourceHandle> reads;
		std::vector<ResourceHandle> writes;
		std::vector<ResourceHandle> readAndWrites;
		std::vector<uint32_t> dependsOnPasses;
		std::vector<uint32_t> successorPasses;
		uint32_t inDegree = 0;
		bool used = false; // for culling
	};

	struct CompiledPlan {
		std::vector<uint32_t> sortedPasses;
		std::vector<uint32_t> memBlockMapping;
		std::vector<std::vector<Barrier>> barriers;
	};

	inline uint32_t AlignUp(uint32_t value, uint32_t alignment) {
		return(value + alignment - 1) & ~(alignment - 1);
	}

	inline uint32_t BytesPerPixel(Format fmt) {
		switch (fmt) {
		case Format::RGBA8:
			return 4;
		case Format::D32F:
			return 4;
		default:
			RFCT_CRITICAL("Unknown format");
		}
	}

	inline uint32_t AllocSize(const ResourceDesc& desc) {
		uint32_t raw = desc.width * desc.height * BytesPerPixel(desc.format);
		return AlignUp(raw, kPlacementAlignment);
	}

	class FrameGraph {
	public:
		ResourceHandle CreateResource(const ResourceDesc& desc);
		ResourceHandle ImportResource(const ResourceDesc& desc, ResourceState initialState = ResourceState::Undefined);
		template <typename SetupFn, typename ExecFn>
		uint32_t AddPass(const std::string& name, SetupFn&& setup, ExecFn&& exec) {
			m_passes.push_back({ name, std::forward<SetupFn>(setup), std::forward<ExecFn>(exec) });
			uint32_t passIdx = static_cast<uint32_t>(m_passes.size() - 1);
			m_passes.back().Setup(passIdx);
			return passIdx;
		};
		void PreparePresent(ResourceHandle handle);
		void Read(uint32_t passIdx, ResourceHandle handle);
		void Write(uint32_t passIdx, ResourceHandle handle);
		CompiledPlan Compile();
		void Execute(CommandList* cmdList);
		void Reset();
		void ForgetAllResources();
		ResourceDesc& GetResourceDesc(ResourceHandle handle);
	private:
		// building
		void BuildEdges();
		std::vector<uint32_t> TopoSort();
		void Cull(const std::vector<uint32_t>& sortedPasses);
		ResourceState StateForUsage(ResourceHandle h, bool isWrite);
		std::vector<std::vector<Barrier>> ComputeBarriers(const std::vector<uint32_t>& sortedPasses, const std::vector<uint32_t>& blockMapping);
		// execution
		void Execute(const CompiledPlan& plan, CommandList* cmdList);
		void ApplyBarriers(const std::vector<Barrier>& barriers, CommandList* cmdList);
		void ApplyBarrier(const Barrier& barrier, CommandList* cmdList);
		// resource aliasing
		std::vector<Lifetime> ScanLifetimes(const std::vector<uint32_t>& sorted);
		std::vector<uint32_t> AliasResources(const std::vector<Lifetime>& lifetimes);
	private:
		std::vector<RenderPass> m_passes;
		std::vector<ResourceEntry> m_entries;
		ResourceHandle m_presentResource;
	};
}