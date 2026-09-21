#include "graphics_pass.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"
#include "world_p/render_data.h"
#include "render_objects.h"
#include "renderer_components.h"
#include "renderer_p/frame/frame_data.h"
#include "world_p/scene.h"
#include "world_p/player/player_animations.h"
#include "renderer_p/frame_graph/frame_graph.h"
#include "context.h"

void rfct::RfctScenePass::CreatePassResources(RfctPipelineManager& pipelineManager, vk::Device device) {
    RFCT_PROFILE_FUNCTION();
    // pass
    RfctRenderPass::RfctRenderPassSpec passSpec;
    passSpec.colorAttachmentDesc.format = vk::Format::eB8G8R8A8Unorm;
    passSpec.colorAttachmentDesc.samples = vk::SampleCountFlagBits::e4;
    passSpec.colorAttachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
    passSpec.colorAttachmentDesc.storeOp = vk::AttachmentStoreOp::eDontCare;
    passSpec.colorAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    passSpec.colorAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    passSpec.colorAttachmentDesc.initialLayout = vk::ImageLayout::eUndefined;
    passSpec.colorAttachmentDesc.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    passSpec.resolveAttachmentDesc.format = vk::Format::eB8G8R8A8Unorm;
    passSpec.resolveAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
    passSpec.resolveAttachmentDesc.loadOp = vk::AttachmentLoadOp::eDontCare;
    passSpec.resolveAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
    passSpec.resolveAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    passSpec.resolveAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    passSpec.resolveAttachmentDesc.initialLayout = vk::ImageLayout::eUndefined;
    passSpec.resolveAttachmentDesc.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    m_renderPass.CreateRenderPass(passSpec, device);

    // pipeline
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

    m_pipelineRef = pipelineManager.CreatePipeline(scenePipeline, m_renderPass.GetPass(), device);
}
void rfct::RfctScenePass::DestroyPassResources(vk::Device device) {
    m_pipelineRef->DestroyPipeline(device);
}

void rfct::RfctScenePass::RecordCommandBuffer(frameContext* ctx, RfctSwapChain& swapChainWrapper, frameSyncDataTemp& frameSyncDataTemp, vk::Framebuffer framebuffer) {
    RFCT_PROFILE_FUNCTION();
    RfctFrameGraphPerFrameResources& frameGraphOwnedCurrentFrameResources = ctx->frameGraph->GetFrameResources(ctx->frameInFlightIndex);

    const renderData& renderdata = ctx->scene->getRenderData();
    vk::CommandBuffer commandBuffer = frameSyncDataTemp.m_sceneCommandBuffer.get();

    commandBuffer.reset({});
    vk::CommandBufferBeginInfo beginInfo = {};
    RFCT_VULKAN_CHECK(commandBuffer.begin(beginInfo));

    std::array<vk::ClearValue, 1> clearValues = {};
    clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});

    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = m_renderPass.GetPass();
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

        vk::DescriptorSet sets[] = { frameGraphOwnedCurrentFrameResources.GetSceneUniformBuffer().GetCameraDescSet(), renderdata.m_DescriptorSetStatic.get()};
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineRef->GetPipelineLayout(), 0, sets, {});

        commandBuffer.draw(renderdata.m_verticesCountStaticObj, 1, 0, 0);
    }

    vk::DescriptorSet sets[] = { frameGraphOwnedCurrentFrameResources.GetSceneUniformBuffer().GetCameraDescSet(), renderdata.m_DescriptorSetsDynamic[ctx->frameInFlightIndex].get() };
    if (renderdata.m_verticesCountDynamicObj) {

        vk::Buffer vertexBuffers[] = { renderdata.m_VertexBufferDynamic[ctx->frameInFlightIndex]->buffer };
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineRef->GetPipelineLayout(), 0, sets, {});

        commandBuffer.draw(renderdata.m_verticesCountDynamicObj, 1, 0, 0);
    }

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineRef->GetPipelineLayout(), 0, sets, {});
    playerAnimations::get().drawPlayer(commandBuffer);
    objectSystems::get().customDrawObjects(commandBuffer, ctx);

    commandBuffer.endRenderPass();
    RFCT_VULKAN_CHECK(commandBuffer.end());
}