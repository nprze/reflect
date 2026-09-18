#include "frame_graph.h"
#include <numeric>

uint32_t MinUint(uint32_t a, uint32_t b) { return a < b ? a : b; }
uint32_t MaxUint(uint32_t a, uint32_t b) { return a > b ? a : b; }

rfct::RfctFGPassHandle rfct::RfctFrameGraph::AddPass(const std::string& name, std::function<void(RfctFGPassHandle)>&& setup, std::function<void(CommandList*)>&& exec) {
	m_passes.push_back({ name, std::forward<std::function<void(RfctFGPassHandle)>>(setup), std::forward<std::function<void(CommandList*)>>(exec) });
    RfctFGPassHandle pass = { static_cast<uint32_t>(m_passes.size() - 1) };
    m_passes.back().Setup(pass);
	return pass;
}

rfct::RfctFGResourceHandle rfct::RfctFrameGraph::ImportResource(const RfctFGResourceDesc& desc, RfctFGResourceState initialState) {
	m_entries.push_back({ desc, { }, initialState });
	return { static_cast<uint32_t>(m_entries.size() - 1) };
}

void rfct::RfctFrameGraph::Read(RfctFGPassHandle pass, RfctFGResourceHandle handle) {
    RfctFGResourceVersion& ver = m_entries[handle.resourceIndex].versions.back();
    if (ver.HasWriter()) {
        m_passes[pass.passIndex].dependsOnPasses.push_back(ver.writerPass);
    }
    ver.readerPasses.push_back({ pass });
    // save for sync
    m_passes[pass.passIndex].reads.push_back({ handle });
}

void rfct::RfctFrameGraph::Write(RfctFGPassHandle pass, RfctFGResourceHandle handle) {
    if (m_entries[handle.resourceIndex].versions.size() == 0)
        m_entries[handle.resourceIndex].versions.push_back({});
    RfctFGResourceVersion& ver = m_entries[handle.resourceIndex].versions.back();
    if (ver.HasWriter()) {
        m_passes[pass.passIndex].dependsOnPasses.push_back(ver.writerPass);
    }
    // wait for all readers to finish
    for (RfctFGPassHandle reader : ver.readerPasses) {
        m_passes[pass.passIndex].dependsOnPasses.push_back(reader);
    }
    if (m_entries[handle.resourceIndex].versions.size() != 1)
        m_entries[handle.resourceIndex].versions.push_back({});
    m_entries[handle.resourceIndex].versions.back().writerPass = pass;
    // save for sync
    m_passes[pass.passIndex].writes.push_back(handle);
}

rfct::RfctFGCompiledPlan rfct::RfctFrameGraph::Compile() {
    BuildEdges();
    std::vector<RfctFGPassHandle> sortedPasses = TopoSort();
    Cull(sortedPasses);
    std::vector<RfctFGResourceLifetime> lifetimes = ScanLifetimes(sortedPasses);
    std::vector<RfctFGMemBlockHandle> physicalBlockMapping = AliasResources(lifetimes);
    std::vector<std::vector<RfctFGBarrier>> barriers = ComputeBarriers(sortedPasses, physicalBlockMapping);
    return { std::move(sortedPasses), std::move(physicalBlockMapping), std::move(barriers) };
}

void rfct::RfctFrameGraph::Execute(CommandList* cmdList) {
    Execute(Compile(), cmdList);
    Reset();
}

void rfct::RfctFrameGraph::Reset() {
    m_passes.clear();
    for (RfctFGResourceEntry& entry : m_entries) {
        entry.versions.clear();
    }
}

void rfct::RfctFrameGraph::ForgetAllResources() {
    m_entries.clear();
}

void rfct::RfctFrameGraph::BuildEdges() {
    for (uint32_t i = 0; i < m_passes.size(); i++) {
        std::unordered_set<uint32_t> seen;
        for (RfctFGPassHandle dep : m_passes[i].dependsOnPasses) {
            if (seen.insert(dep.passIndex).second) {
                m_passes[dep.passIndex].successorPasses.push_back({ i });
                m_passes[i].inDegree++;
            }
        }
    }
}

std::vector<rfct::RfctFGPassHandle> rfct::RfctFrameGraph::TopoSort() {
    std::queue<RfctFGPassHandle> queue;
    std::vector<uint32_t> inDeg(m_passes.size());
    // start with passes with no deps
    for (uint32_t i = 0; i < m_passes.size(); i++) {
        inDeg[i] = m_passes[i].inDegree;
        if (inDeg[i] == 0) {
            queue.push({ i });
        }
    }
    std::vector<RfctFGPassHandle> order;
    order.reserve(m_passes.size());
    while (!queue.empty()) {
        RfctFGPassHandle cur = queue.front();
        queue.pop();
        order.push_back(cur);
        for (RfctFGPassHandle succ : m_passes[cur.passIndex].successorPasses) {
            if (--inDeg[succ.passIndex] == 0)
                queue.push(succ);
        }
    }
    // the graph has a cycle
    RFCT_ASSERT(order.size() == m_passes.size(), "Cycle detected!")
        return order;
}

void rfct::RfctFrameGraph::Cull(const std::vector<RfctFGPassHandle>& sortedPasses) {
    // marks used passes
    if (sortedPasses.empty())
        return;
    m_passes[sortedPasses.back().passIndex].used = true; // last is the final output
    for (int32_t i = static_cast<uint32_t>(sortedPasses.size() - 1); i >= 0; i--) {
        if (!m_passes[sortedPasses[i].passIndex].used)
            continue;
        for (RfctFGPassHandle dep : m_passes[sortedPasses[i].passIndex].dependsOnPasses)
            m_passes[dep.passIndex].used = true;
    }
}

rfct::RfctFGResourceState rfct::RfctFrameGraph::StateForUsage(RfctFGResourceHandle h, bool isWrite) {
    if (isWrite) {
        RFCT_ASSERT(m_entries[h.resourceIndex].desc.format == RfctFGFormat::RGBA8); // TODO: depth is not supported, let me cook
        return RfctFGResourceState::RenderTarget;
    }
    return RfctFGResourceState::ShaderRead;
}

std::vector<std::vector<rfct::RfctFGBarrier>> rfct::RfctFrameGraph::ComputeBarriers(const std::vector<RfctFGPassHandle>& sortedPasses, const std::vector<RfctFGMemBlockHandle>& blockMapping) {
    std::vector<std::vector<RfctFGBarrier>> result(sortedPasses.size());
    // blockOwner[block] -> resource currently occupying the block
    std::vector<uint32_t> blockOwner;
    {
        uint32_t maxBlock = 0;
        for (RfctFGMemBlockHandle m : blockMapping)
            if (m.IsValid())
                maxBlock = { MaxUint(maxBlock, m.blockIndex + 1) };
        blockOwner.assign(maxBlock, UINT32_MAX);
    }
    for (uint32_t orderIdx = 0; orderIdx < sortedPasses.size(); orderIdx++) {
        RfctFGPassHandle pass = sortedPasses[orderIdx];
        if (!m_passes[pass.passIndex].used)
            continue;
        std::vector<std::pair<RfctFGResourceHandle, bool>> unique;  // <handle, isWrite>
        std::unordered_set<uint32_t> seenResources;
        for (RfctFGResourceHandle& h : m_passes[pass.passIndex].reads) {
            if (seenResources.insert(h.resourceIndex).second) {
                unique.push_back({ h, false });
            }
        }
        for (RfctFGResourceHandle& h : m_passes[pass.passIndex].writes) {
            if (seenResources.insert(h.resourceIndex).second) {
                unique.push_back({ h, true });
            }
        }
        /*
        // aliasing barriers
        for (auto& [h, _] : unique) {
            RfctFGMemBlockHandle block = blockMapping[h.resourceIndex];
            if (!block.IsValid())
                continue;
            if (blockOwner[block.blockIndex] != h.resourceIndex) {
                result[orderIdx].push_back({ h.resourceIndex, RfctFGResourceState::Undefined, RfctFGResourceState::Undefined, true, blockOwner[block] });
            }
            blockOwner[block.blockIndex] = h.resourceIndex;
        }
        */
        // state barriers
        for (auto& [h, isWrite] : unique) {
            RfctFGResourceState needed = StateForUsage(h, isWrite);
            if (m_entries[h.resourceIndex].currentState != needed) {
                result[orderIdx].push_back({ h.resourceIndex, m_entries[h.resourceIndex].currentState, needed });
                m_entries[h.resourceIndex].currentState = needed;
            }
        }
    }
    return result;
}

void rfct::RfctFrameGraph::Execute(const RfctFGCompiledPlan& plan, CommandList* cmdList) {
    for (uint32_t orderIdx = 0; orderIdx < plan.sortedPasses.size(); orderIdx++) {
        RfctFGPassHandle pass = plan.sortedPasses[orderIdx];
        if (!m_passes[pass.passIndex].used)
            continue;
        ApplyBarriers(plan.barriers[orderIdx], cmdList);
        m_passes[pass.passIndex].Execute(cmdList);
    }
    RfctFGPassHandle lastPass = plan.sortedPasses[plan.sortedPasses.size() - 1];
    RfctFGRenderPass pass = m_passes[lastPass.passIndex];
    RFCT_ASSERT(m_passes[lastPass.passIndex].used && (pass.writes.size() == 0));
    RfctFGResourceEntry& presentResource = m_entries[pass.writes[0].resourceIndex];
    RfctFGBarrier barrier = { pass.writes[0].resourceIndex, presentResource.currentState, RfctFGResourceState::Present };
    ApplyBarrier(barrier, cmdList);
    presentResource.currentState = RfctFGResourceState::Present;
}

void rfct::RfctFrameGraph::ApplyBarriers(const std::vector<RfctFGBarrier>& barriers, CommandList* cmdList) {
    for (const RfctFGBarrier& b : barriers) {
        ApplyBarrier(b, cmdList);
    }
}

void rfct::RfctFrameGraph::ApplyBarrier(const RfctFGBarrier& barrier, CommandList* cmdList) {
    // the vulkan impl of apply barrier
}

std::vector<rfct::RfctFGResourceLifetime> rfct::RfctFrameGraph::ScanLifetimes(const std::vector<RfctFGPassHandle>& sorted) {
    std::vector<RfctFGResourceLifetime> life(m_entries.size());
    for (uint32_t order = 0; order < sorted.size(); order++) {
        uint32_t passIdx = sorted[order].passIndex;
        if (!m_passes[passIdx].used)
            continue;
        for (auto& h : m_passes[passIdx].reads) {
            life[h.resourceIndex].firstUsePass = { MinUint(life[h.resourceIndex].firstUsePass.passIndex, order) };
            life[h.resourceIndex].lastUsePass = { MaxUint(life[h.resourceIndex].lastUsePass.passIndex, order) };
        }
        for (auto& h : m_passes[passIdx].writes) {
            life[h.resourceIndex].firstUsePass = { MinUint(life[h.resourceIndex].firstUsePass.passIndex, order) };
            life[h.resourceIndex].lastUsePass = { MaxUint(life[h.resourceIndex].lastUsePass.passIndex, order) };
        }
    }
    return life;
}

std::vector<rfct::RfctFGMemBlockHandle> rfct::RfctFrameGraph::AliasResources(const std::vector<RfctFGResourceLifetime>& lifetimes) {
    // TODO: Aliasing
    std::vector<RfctFGMemBlockHandle> memBlockMapping(m_entries.size(), { UINT32_MAX });
    return memBlockMapping;
}
