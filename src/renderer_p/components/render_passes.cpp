#include "render_passes.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"
#include "world_p/render_data.h"
#include "render_objects.h"
#include "renderer_components.h"
#include "renderer_p/frame/frame_data.h"

void rfct::RfctScenePass::CreatePassResources(vk::RenderPass renderPass, RfctPipelineManager& pipelineManager, vk::Device device) {
    RFCT_PROFILE_FUNCTION();
	RfctRenderPipeline::RfctRenderPipelineSpec scenePipeline;
    scenePipeline.vertexShaderPath = "shaders/basic/basic_vert.spv";
    scenePipeline.fragmentShaderPath = "shaders/basic/basic_frag.spv";
    scenePipeline.MSAA4x = true;
    auto attributeDesc = Vertex::getAttributeDescriptions();
    std::vector<vk::VertexInputAttributeDescription> vecAttributeDesc;
    vecAttributeDesc.reserve(attributeDesc.size());
    for (uint32_t i = 0; i < attributeDesc.size(); i++)
        vecAttributeDesc.push_back(attributeDesc[i]);
    scenePipeline.vertexInputAttributeDescriptions = vecAttributeDesc;
    scenePipeline.vertexInputBindingDescription = Vertex::getBindingDescription();
    std::vector<vk::DescriptorSetLayout> descSetLayouts = { RfctUniformBuffer::GetUniformDescriptorSetLayout(device), renderData::getDescriptorSetLayout() };
    scenePipeline.descriptorSetLayouts = descSetLayouts;

    m_pipelineRef = pipelineManager.CreatePipeline(scenePipeline, renderPass, device);
}
void rfct::RfctScenePass::RecordCommandBuffer(frameContext* ctx, RfctSwapChain& swapChainWrapper, RfctFrameSyncData& RfctFrameSyncData, vk::Framebuffer framebuffer, vk::RenderPass renderPass) {
    RFCT_PROFILE_FUNCTION();
    const renderData& renderdata = ctx->scene->getRenderData();
    vk::CommandBuffer commandBuffer = RfctFrameSyncData.m_sceneCommandBuffer.get();

    commandBuffer.reset({});
    vk::CommandBufferBeginInfo beginInfo = {};
    RFCT_VULKAN_CHECK(commandBuffer.begin(beginInfo));

    std::array<vk::ClearValue, 1> clearValues = {};
    clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});

    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    renderPassInfo.renderArea.extent = swapChainWrapper.GetExtent();
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelineRef->GetPipeline());

    vk::Viewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(renderPassInfo.renderArea.extent.width);
    viewport.height = static_cast<float>(renderPassInfo.renderArea.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    commandBuffer.setViewport(0, viewport);

    vk::Rect2D scissor = {};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = renderPassInfo.renderArea.extent;
    commandBuffer.setScissor(0, scissor);

    vk::DeviceSize offsets[] = { 0 };
    // Camera Descriptor
    if (renderdata.m_verticesCountStaticObj) {
        vk::Buffer vertexBuffers[] = { renderdata.m_VertexBufferStatic.m_Buffer.buffer };
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

        vk::DescriptorSet sets[] = { RfctFrameSyncData.getCameraUboDescSet(), renderdata.m_DescriptorSetStatic.get() };
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0, sets, {});

        commandBuffer.draw(renderdata.m_verticesCountStaticObj, 1, 0, 0);
    }

    if (renderdata.m_verticesCountDynamicObj) {

        vk::Buffer vertexBuffers[] = { renderdata.m_VertexBufferDynamic[ctx->frame]->buffer };
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

        vk::DescriptorSet sets[] = { RfctFrameSyncData.getCameraUboDescSet(), renderdata.m_DescriptorSetsDynamic[ctx->frame].get() };
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0, sets, {});

        commandBuffer.draw(renderdata.m_verticesCountDynamicObj, 1, 0, 0);
    }

    vk::DescriptorSet sets[] = { RfctFrameSyncData.getCameraUboDescSet(), renderdata.m_DescriptorSetsDynamic[ctx->frame].get() };
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0, sets, {});
    playerAnimations::get().drawPlayer(commandBuffer);
    objectSystems::get().customDrawObjects(commandBuffer, ctx);

    commandBuffer.endRenderPass();
    RFCT_VULKAN_CHECK(commandBuffer.end());
}