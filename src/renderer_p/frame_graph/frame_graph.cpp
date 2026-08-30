#include "frame_graph.h"
#include <unordered_set>
#include <queue>
#include <numeric>

// helper functions
uint32_t minUint(uint32_t a, uint32_t b) { return a < b ? a : b; }
uint32_t maxUint(uint32_t a, uint32_t b) { return a > b ? a : b; }

rfct::ResourceHandle rfct::FrameGraph::CreateResource(const ResourceDesc& desc) {
    RFCT_ASSERT(false); // TODO: Make it possible for fg to own resources
    m_entries.push_back({ desc, { }, ResourceState::Undefined, false });
    return { static_cast<uint32_t>(m_entries.size() - 1) };
}

rfct::ResourceHandle rfct::FrameGraph::ImportResource(const ResourceDesc& desc, ResourceState initialState) {
    m_entries.push_back({ desc, { }, initialState, true });
    return { static_cast<uint32_t>(m_entries.size() - 1) };
}

void rfct::FrameGraph::PreparePresent(ResourceHandle handle) {
    RFCT_ASSERT(handle.IsValid());
    m_presentResource = handle;
}

void rfct::FrameGraph::Read(uint32_t passIdx, ResourceHandle handle) {
    ResourceVersion& ver = m_entries[handle.resourceIndex].versions.back();
    if (ver.HasWriter()) {
        m_passes[passIdx].dependsOnPasses.push_back(ver.writerPassIndex);
    }
    ver.readerPassIndices.push_back(passIdx);
    // save for sync
    m_passes[passIdx].reads.push_back(handle);
}

void rfct::FrameGraph::Write(uint32_t passIdx, ResourceHandle handle) {
    if (m_entries[handle.resourceIndex].versions.size() == 0)
        m_entries[handle.resourceIndex].versions.push_back({});
    ResourceVersion& ver = m_entries[handle.resourceIndex].versions.back();
    if (ver.HasWriter()) {
        m_passes[passIdx].dependsOnPasses.push_back(ver.writerPassIndex);
    }
    // wait for all readers to finish
    for (uint32_t reader : ver.readerPassIndices) {
        m_passes[passIdx].dependsOnPasses.push_back(reader);
    }
    if (m_entries[handle.resourceIndex].versions.size() != 1)
        m_entries[handle.resourceIndex].versions.push_back({});
    m_entries[handle.resourceIndex].versions.back().writerPassIndex = passIdx;
    // save for sync
    m_passes[passIdx].writes.push_back(handle);
}

void rfct::FrameGraph::BuildEdges() {
    for (uint32_t i = 0; i < m_passes.size(); i++) {
        std::unordered_set<uint32_t> seen;
        for (uint32_t dep : m_passes[i].dependsOnPasses) {
            if (seen.insert(dep).second) {
                m_passes[dep].successorPasses.push_back(i);
                m_passes[i].inDegree++;
            }
        }
    }
}

std::vector<uint32_t> rfct::FrameGraph::TopoSort() {
    std::queue<uint32_t> queue;
    std::vector<uint32_t> inDeg(m_passes.size());
    // start with passes with no deps
    for (uint32_t i = 0; i < m_passes.size(); i++) {
        inDeg[i] = m_passes[i].inDegree;
        if (inDeg[i] == 0) {
            queue.push(i);
        }
    }
    std::vector<uint32_t> order;
    order.reserve(m_passes.size());
    while (!queue.empty()) {
        uint32_t cur = queue.front();
        queue.pop();
        order.push_back(cur);
        for (uint32_t succ : m_passes[cur].successorPasses) {
            if (--inDeg[succ] == 0)
                queue.push(succ);
        }
    }
    // the graph has a cycle
    RFCT_ASSERT(order.size() == m_passes.size(), "Cycle detected!")
    return order;
}

void rfct::FrameGraph::Cull(const std::vector<uint32_t>& sortedPasses) {
    // marks used passes
    if (sortedPasses.empty())
        return;
    m_passes[sortedPasses.back()].used = true; // last is the final output
    for (int32_t i = static_cast<uint32_t>(sortedPasses.size() - 1); i >= 0; i--) {
        if (!m_passes[sortedPasses[i]].used)
            continue;
        for (uint32_t dep : m_passes[sortedPasses[i]].dependsOnPasses)
            m_passes[dep].used = true;
    }
}

rfct::ResourceState rfct::FrameGraph::StateForUsage(ResourceHandle h, bool isWrite) {
    if (isWrite) {
        RFCT_ASSERT(m_entries[h.resourceIndex].desc.format == Format::RGBA8); // TODO: depth is not supported, let me cook
        return ResourceState::RenderTarget;
    }
    return ResourceState::ShaderRead;
}

std::vector<std::vector<rfct::Barrier>> rfct::FrameGraph::ComputeBarriers(const std::vector<uint32_t>& sortedPasses, const std::vector<uint32_t>& blockMapping) {
    std::vector<std::vector<Barrier>> result(sortedPasses.size());
    // blockOwner[block] -> resource currently occupying the block
    std::vector<uint32_t> blockOwner;
    {
        uint32_t maxBlock = 0;
        for (uint32_t m : blockMapping)
            if (m != UINT32_MAX)
                maxBlock = maxUint(maxBlock, m + 1);
        blockOwner.assign(maxBlock, UINT32_MAX);
    }
    for (uint32_t orderIdx = 0; orderIdx < sortedPasses.size(); orderIdx++) {
        uint32_t passIdx = sortedPasses[orderIdx];
        if (!m_passes[passIdx].used)
            continue;
        std::vector<std::pair<ResourceHandle, bool>> unique;  // <handle, isWrite>
        std::unordered_set<uint32_t> seenResources;
        for (ResourceHandle& h : m_passes[passIdx].reads) {
            if (seenResources.insert(h.resourceIndex).second) {
                unique.push_back({ h, false });
            }
        }
        for (ResourceHandle& h : m_passes[passIdx].writes) {
            if (seenResources.insert(h.resourceIndex).second) {
                unique.push_back({ h, true });
            }
        }
        // aliasing barriers
        for (auto& [h, _] : unique) {
            uint32_t block = blockMapping[h.resourceIndex];
            if (block == UINT32_MAX)
                continue;
            if (blockOwner[block] != h.resourceIndex) {
                result[orderIdx].push_back({ h.resourceIndex, ResourceState::Undefined, ResourceState::Undefined, true, blockOwner[block] });
            }
            blockOwner[block] = h.resourceIndex;
        }
        // state barriers
        for (auto& [h, isWrite] : unique) {
            ResourceState needed = StateForUsage(h, isWrite);
            if (m_entries[h.resourceIndex].currentState != needed) {
                result[orderIdx].push_back({ h.resourceIndex, m_entries[h.resourceIndex].currentState, needed });
                m_entries[h.resourceIndex].currentState = needed;
            }
        }
    }
    return result;
}

void rfct::FrameGraph::ApplyBarriers(const std::vector<Barrier>& barriers, CommandList* cmdList) {
    for (const Barrier& b : barriers) {
        RFCT_ASSERT(!b.isAliasing); // TODO: Support fg owned resources (aliasable)
        ApplyBarrier(b, cmdList);
    }
}

void rfct::FrameGraph::ApplyBarrier(const Barrier& barrier, CommandList* cmdList) {
    CD3DX12_RESOURCE_BARRIER finBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_entries[barrier.resourceIndex].desc.resourcePtr,
        EnumToResState(barrier.oldState), EnumToResState(barrier.newState)
    );
    cmdList->ResourceBarrier(1, &finBarrier);
}

std::vector<rfct::Lifetime> rfct::FrameGraph::ScanLifetimes(const std::vector<uint32_t>& sorted) {
    std::vector<Lifetime> life(m_entries.size());
    for (uint32_t i = 0; i < m_entries.size(); i++) {
        if (m_entries[i].imported)
            life[i].isTransient = false;
    }
    for (uint32_t order = 0; order < sorted.size(); order++) {
        uint32_t passIdx = sorted[order];
        if (!m_passes[passIdx].used)
            continue;
        for (auto& h : m_passes[passIdx].reads) {
            life[h.resourceIndex].firstUsePass = minUint(life[h.resourceIndex].firstUsePass, order);
            life[h.resourceIndex].lastUsePass = maxUint(life[h.resourceIndex].lastUsePass, order);
        }
        for (auto& h : m_passes[passIdx].writes) {
            life[h.resourceIndex].firstUsePass = minUint(life[h.resourceIndex].firstUsePass, order);
            life[h.resourceIndex].lastUsePass = maxUint(life[h.resourceIndex].lastUsePass, order);
        }
    }
    return life;
}

std::vector<uint32_t> rfct::FrameGraph::AliasResources(const std::vector<Lifetime>& lifetimes) {
    std::vector<PhysicalBlock> freeList;
    std::vector<uint32_t> memBlockMapping(m_entries.size(), UINT32_MAX);
    std::vector<uint32_t> indices(m_entries.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](uint32_t a, uint32_t b) {
        return lifetimes[a].firstUsePass < lifetimes[b].firstUsePass;
        });
    for (uint32_t resIdx : indices) {
        if (!lifetimes[resIdx].isTransient)
            continue; // imported are not aliased
        if (lifetimes[resIdx].firstUsePass == UINT32_MAX)
            continue;
        uint32_t needed = AllocSize(m_entries[resIdx].desc);
        bool reused = false;
        for (uint32_t b = 0; b < freeList.size(); b++) {
            if (freeList[b].availAfterPass < lifetimes[resIdx].firstUsePass
                && freeList[b].sizeBytes >= needed) {
                memBlockMapping[resIdx] = b; // reuse
                freeList[b].availAfterPass = lifetimes[resIdx].lastUsePass;
                reused = true;
                break;
            }
        }
        if (!reused) {  // allocate a new physical block
            memBlockMapping[resIdx] = static_cast<uint32_t>(freeList.size());
            freeList.push_back({ needed, lifetimes[resIdx].lastUsePass });
        }
    }
    return memBlockMapping;
}

rfct::CompiledPlan rfct::FrameGraph::Compile() {
    BuildEdges();
    std::vector<uint32_t> sortedPasses = TopoSort();
    Cull(sortedPasses);
    std::vector<Lifetime> lifetimes = ScanLifetimes(sortedPasses);
    std::vector<uint32_t> physicalBlockMapping = AliasResources(lifetimes);
    std::vector<std::vector<Barrier>> barriers = ComputeBarriers(sortedPasses, physicalBlockMapping);
    return { std::move(sortedPasses), std::move(physicalBlockMapping), std::move(barriers) };
}

void rfct::FrameGraph::Execute(const CompiledPlan& plan, CommandList* cmdList) {
    for (uint32_t orderIdx = 0; orderIdx < plan.sortedPasses.size(); orderIdx++) {
        uint32_t passIdx = plan.sortedPasses[orderIdx];
        if (!m_passes[passIdx].used)
            continue;
        ApplyBarriers(plan.barriers[orderIdx], cmdList);
        m_passes[passIdx].Execute(cmdList);
    }
    if (m_presentResource.IsValid()) {
        Barrier barrier = { m_presentResource.resourceIndex, m_entries[m_presentResource.resourceIndex].currentState, ResourceState::Present };
        ApplyBarrier(barrier, cmdList);
        m_entries[m_presentResource.resourceIndex].currentState = ResourceState::Present;
    }
}

void rfct::FrameGraph::Execute(CommandList* cmdList) {
    Execute(Compile(), cmdList);
    Reset();
}

void rfct::FrameGraph::Reset() {
    m_passes.clear();
    for (ResourceEntry& entry : m_entries) {
        entry.versions.clear();
    }
    m_presentResource.resourceIndex = UINT_MAX;
}

void rfct::FrameGraph::ForgetAllResources() {
    m_entries.clear();
}

rfct::ResourceDesc& rfct::FrameGraph::GetResourceDesc(ResourceHandle handle) {
    return m_entries[handle.resourceIndex].desc;
}

DXGI_FORMAT rfct::EnumToResFormat(const Format& format) {
    switch (format) {
    case (Format::RGBA8):
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case (Format::D32F):
        return DXGI_FORMAT_D32_FLOAT;
    default:
        return DXGI_FORMAT_UNKNOWN;
    };
    return DXGI_FORMAT_UNKNOWN;
}

D3D12_RESOURCE_STATES rfct::EnumToResState(rfct::ResourceState state) {
    switch (state) {
    case rfct::ResourceState::RenderTarget:
        return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case rfct::ResourceState::ShaderRead:
        return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    case rfct::ResourceState::Present:
        return D3D12_RESOURCE_STATE_PRESENT;
    }
    rfct_SOFT_ASSERT(false); // Achievement got: How Did We Get Here?
    return D3D12_RESOURCE_STATE_COMMON;
}