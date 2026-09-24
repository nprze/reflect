#include "graphics_pass.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"
#include "world_p/render_data.h"
#include "render_objects.h"
#include "renderer_components.h"
#include "world_p/scene.h"
#include "world_p/player/player_animations.h"
#include "renderer_p/frame_graph/frame_graph.h"
#include "context.h"

void rfct::RfctSceneGraphicsPass::CreatePassResources(RfctRenderPass* renderPass, RfctPipelineManager& pipelineManager, vk::Device device) {
    RFCT_PROFILE_FUNCTION();
    m_renderPassRef = renderPass;
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

    m_pipelineRef = pipelineManager.CreatePipeline(scenePipeline, m_renderPassRef->GetPass(), device);
}

void rfct::RfctSceneGraphicsPass::DestroyPassResources(vk::Device device) {
    m_pipelineRef->DestroyPipeline(device);
}

void rfct::RfctSceneGraphicsPass::RecordCommandBuffer(RfctFrameContext& ctx, frameSyncDataTemp& frameSyncDataTemp, vk::Framebuffer framebuffer) {
    RFCT_PROFILE_FUNCTION();
    RfctFrameGraphPerFrameResources& frameGraphOwnedCurrentFrameResources = ctx.frameGraph->GetFrameResources(ctx.frameInFlightIndex);

    const renderData& renderdata = ctx.scene->getRenderData();
    vk::CommandBuffer commandBuffer = ctx.graphicsCmdBffr;

    commandBuffer.reset({});
    vk::CommandBufferBeginInfo beginInfo = {};
    RFCT_VULKAN_CHECK(commandBuffer.begin(beginInfo));

    std::array<vk::ClearValue, 1> clearValues = {};
    clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});

    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = m_renderPassRef->GetPass();
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    renderPassInfo.renderArea.extent = ctx.imageExtent;
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

void rfct::RfctBloomGraphicsPass::CreateBloomPassResources(RfctRenderPass* gaussianPass, RfctRenderPass* compositePass, RfctPipelineManager& pipelineManager, vk::Device device) {
    RFCT_PROFILE_FUNCTION();
    m_gaussianPass = gaussianPass;
    m_compositePass = compositePass;
    {
        RFCT_PROFILE_SCOPE("Create gaussian pipeline");
        // descriptor set layout
        vk::DescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = 0;
        layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        layoutCreateInfo.bindingCount = 1;
        layoutCreateInfo.pBindings = &layoutBinding;

        m_gaussianPipelineDescLayout = device.createDescriptorSetLayout(layoutCreateInfo).value;

        vk::PushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eFragment;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(RfctBloomGraphicsPass::RfctBloomPushConstants);

        RfctRenderPipeline::RfctRenderPipelineSpec gaussianBlurPipeline;
        gaussianBlurPipeline.vertexShaderPath = "shaders/post_proc/fullscreen_vert.spv";
        gaussianBlurPipeline.fragmentShaderPath = "shaders/post_proc/gaussian_blur_frag.spv";
        gaussianBlurPipeline.MSAA4x = true;
        gaussianBlurPipeline.enableVetexBinding = false;
        gaussianBlurPipeline.descriptorSetLayouts = { m_gaussianPipelineDescLayout };
        gaussianBlurPipeline.pushConstantRanges = { pushConstantRange };
        gaussianBlurPipeline.enableColorBlend = true;

        m_gaussianPipelineRef = pipelineManager.CreatePipeline(gaussianBlurPipeline, m_gaussianPass->GetPass(), device);
    }
    {
        RFCT_PROFILE_SCOPE("Create composite pipeline");
        // descriptor set layout
        vk::DescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = 0;
        layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        vk::DescriptorSetLayoutBinding layoutBinding1 = {};
        layoutBinding1.binding = 1;
        layoutBinding1.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        layoutBinding1.descriptorCount = 1;
        layoutBinding1.stageFlags = vk::ShaderStageFlagBits::eFragment;

        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { layoutBinding, layoutBinding1 };
        layoutCreateInfo.bindingCount = 2;
        layoutCreateInfo.pBindings = bindings.data();

        m_compositePipelineDescLayout = device.createDescriptorSetLayout(layoutCreateInfo).value;

        RfctRenderPipeline::RfctRenderPipelineSpec compositePipeline;
        compositePipeline.vertexShaderPath = "shaders/post_proc/fullscreen_vert.spv";
        compositePipeline.fragmentShaderPath = "shaders/post_proc/composite_frag.spv";
        compositePipeline.MSAA4x = true;
        compositePipeline.enableVetexBinding = false;
        compositePipeline.descriptorSetLayouts = { m_compositePipelineDescLayout };
        compositePipeline.enableColorBlend = true;

        m_compositePipelineRef = pipelineManager.CreatePipeline(compositePipeline, m_compositePass->GetPass(), device);
    }
    {
        RFCT_PROFILE_SCOPE("Create sampler");
        vk::SamplerCreateInfo samplerInfo{};
        samplerInfo.magFilter = vk::Filter::eLinear;
        samplerInfo.minFilter = vk::Filter::eLinear;

        samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;

        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;

        samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = vk::CompareOp::eAlways;

        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

        m_imageSampler = device.createSampler(samplerInfo).value;
    }
    {
        RFCT_PROFILE_SCOPE("Allocate descriptor sets");
        vk::DescriptorPoolSize poolSize = {};
        poolSize.type = vk::DescriptorType::eCombinedImageSampler;
        poolSize.descriptorCount = RFCT_FRAMES_IN_FLIGHT * (1 + 1 + 1 + 2);

        vk::DescriptorPoolCreateInfo poolInfo = {};
        poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = RFCT_FRAMES_IN_FLIGHT * 3;

        m_descriptorPool = device.createDescriptorPool(poolInfo).value;
        {
            std::array<vk::DescriptorSetLayout, RFCT_FRAMES_IN_FLIGHT> sets = {};
            for (uint32_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; ++i) { sets[i] = m_gaussianPipelineDescLayout; }
            vk::DescriptorSetAllocateInfo allocInfo(m_descriptorPool, RFCT_FRAMES_IN_FLIGHT, sets.data());
            m_gaussian1ImageDescriptorSet = std::move(device.allocateDescriptorSets(allocInfo).value);
        }
        {
            std::array<vk::DescriptorSetLayout, RFCT_FRAMES_IN_FLIGHT> sets = {};
            for (uint32_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; ++i) { sets[i] = m_gaussianPipelineDescLayout; }
            vk::DescriptorSetAllocateInfo allocInfo(m_descriptorPool, RFCT_FRAMES_IN_FLIGHT, sets.data());
            m_gaussian2ImageDescriptorSet = std::move(device.allocateDescriptorSets(allocInfo).value);
        }
        {

            std::array<vk::DescriptorSetLayout, RFCT_FRAMES_IN_FLIGHT> sets = {};
            for (uint32_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; ++i) { sets[i] = m_compositePipelineDescLayout; }
            vk::DescriptorSetAllocateInfo allocInfo(m_descriptorPool, RFCT_FRAMES_IN_FLIGHT, sets.data());
            m_compositeImageDescriptorSet = std::move(device.allocateDescriptorSets(allocInfo).value);
        }
    }
}

void rfct::RfctBloomGraphicsPass::UpdateGaussian1DescSets(uint32_t frameInFlightIndex, vk::ImageView imageView, vk::Device device) {
    vk::DescriptorImageInfo imageInfo;

    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.imageView = imageView;
    imageInfo.sampler = m_imageSampler;

    vk::WriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.dstSet = m_gaussian1ImageDescriptorSet[frameInFlightIndex];
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pImageInfo = &imageInfo;

    device.updateDescriptorSets({ writeDescriptorSet }, nullptr);
}

void rfct::RfctBloomGraphicsPass::UpdateGaussian2DescSets(uint32_t frameInFlightIndex, vk::ImageView imageView, vk::Device device) {
    vk::DescriptorImageInfo imageInfo;

    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.imageView = imageView;
    imageInfo.sampler = m_imageSampler;

    vk::WriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.dstSet = m_gaussian2ImageDescriptorSet[frameInFlightIndex];
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pImageInfo = &imageInfo;

    device.updateDescriptorSets({ writeDescriptorSet }, nullptr);
}

void rfct::RfctBloomGraphicsPass::UpdateCompositeDescSets(uint32_t frameInFlightIndex, vk::ImageView imageView0, vk::ImageView imageView1, vk::Device device) {
    // composite
    vk::DescriptorImageInfo imageInfo0;
    imageInfo0.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo0.imageView = imageView0;
    imageInfo0.sampler = m_imageSampler;

    vk::WriteDescriptorSet writeDescriptorSet0 = {};
    writeDescriptorSet0.dstSet = m_compositeImageDescriptorSet[frameInFlightIndex];
    writeDescriptorSet0.dstBinding = 0;
    writeDescriptorSet0.dstArrayElement = 0;
    writeDescriptorSet0.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    writeDescriptorSet0.descriptorCount = 1;
    writeDescriptorSet0.pImageInfo = &imageInfo0;

    vk::DescriptorImageInfo imageInfo1;
    imageInfo1.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo1.imageView = imageView1;
    imageInfo1.sampler = m_imageSampler;

    vk::WriteDescriptorSet writeDescriptorSet1 = {};
    writeDescriptorSet1.dstSet = m_compositeImageDescriptorSet[frameInFlightIndex];
    writeDescriptorSet1.dstBinding = 1;
    writeDescriptorSet1.dstArrayElement = 0;
    writeDescriptorSet1.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    writeDescriptorSet1.descriptorCount = 1;
    writeDescriptorSet1.pImageInfo = &imageInfo1;

    std::array<vk::WriteDescriptorSet, 2> writeSets = { writeDescriptorSet0, writeDescriptorSet1 };
    device.updateDescriptorSets(writeSets, nullptr);
}

constexpr uint32_t bloomMultiply = 3;
void rfct::RfctBloomGraphicsPass::RecordCommandBuffer(RfctFrameContext& ctx) {
    RFCT_PROFILE_FUNCTION();
    vk::CommandBuffer commandBuffer = ctx.graphicsCmdBffr;
    imageManager.GetSceneImage(imageIndex).TransformLayoutAsync(vk::ImageLayout::eShaderReadOnlyOptimal, commandBuffer);

    {
        // bloom 0 pipeline
        vk::RenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = imageManager.GetBloom2Image(imageIndex).m_frameBuffer.get();
        renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
        renderPassInfo.renderArea.extent = swapChain.GetExtent();
        renderPassInfo.clearValueCount = 1;
        vk::ClearValue clearColor = {};
        clearColor.color = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));
        renderPassInfo.pClearValues = &clearColor;

        commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

        vk::Viewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChain.GetExtent().width);
        viewport.height = static_cast<float>(swapChain.GetExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        commandBuffer.setViewport(0, viewport);
        vk::Rect2D scissor = {};
        scissor.offset = vk::Offset2D{ 0, 0 };
        scissor.extent = swapChain.GetExtent();
        commandBuffer.setScissor(0, scissor);

        // Descriptors
        vk::DescriptorSet descSets[] = { m_gaussian1SceneImageDescriptorSet[imageIndex].get() };
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_gaussianPipeline.m_pipelineLayout, 0, descSets, {});

        gaussianPushConstants pc;
        pc.dir = glm::vec2(1.f, 0.f);
        pc.res = swapChain.GetExtent().width / bloomMultiply;

        commandBuffer.pushConstants(
            m_gaussianPipeline.m_pipelineLayout,
            vk::ShaderStageFlagBits::eFragment,
            0,
            sizeof(gaussianPushConstants),
            &pc
        );

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_gaussianPipeline.m_pipeline.get());
        commandBuffer.draw(3, 1, 0, 0);
        commandBuffer.endRenderPass();
    }
    {
        // bloom 1 pipeline
        imageManager.GetBloom2Image(imageIndex).TransformLayoutAsync(vk::ImageLayout::eShaderReadOnlyOptimal, commandBuffer);

        vk::RenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = imageManager.GetBloom1Image(imageIndex).m_frameBuffer.get();
        renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
        renderPassInfo.renderArea.extent = swapChain.GetExtent();
        renderPassInfo.clearValueCount = 1;
        vk::ClearValue clearColor = {};
        clearColor.color = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));
        renderPassInfo.pClearValues = &clearColor;

        commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

        vk::Viewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChain.GetExtent().width);
        viewport.height = static_cast<float>(swapChain.GetExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        commandBuffer.setViewport(0, viewport);
        vk::Rect2D scissor = {};
        scissor.offset = vk::Offset2D{ 0, 0 };
        scissor.extent = swapChain.GetExtent();
        commandBuffer.setScissor(0, scissor);

        // Descriptors
        vk::DescriptorSet descSets[] = { m_gaussian2SceneImageDescriptorSet[imageIndex].get() };
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_gaussianPipeline.m_pipelineLayout, 0, descSets, {});

        gaussianPushConstants pc;
        pc.dir = glm::vec2(0.f, 1.f);
        pc.res = swapChain.GetExtent().height / bloomMultiply;

        commandBuffer.pushConstants(
            m_gaussianPipeline.m_pipelineLayout,
            vk::ShaderStageFlagBits::eFragment,
            0,
            sizeof(gaussianPushConstants),
            &pc
        );

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_gaussianPipeline.m_pipeline.get());
        commandBuffer.draw(3, 1, 0, 0);
        commandBuffer.endRenderPass();
    }
    {
        // composite pipeline
        imageManager.GetBloom1Image(imageIndex).TransformLayoutAsync(vk::ImageLayout::eShaderReadOnlyOptimal, commandBuffer);

        vk::RenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.renderPass = imageManager.GetpresentToColorAttachmentRenderPass();
        renderPassInfo.framebuffer = imageManager.GetSwapChainImage(imageIndex).m_frameBuffer.get();
        renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
        renderPassInfo.renderArea.extent = swapChain.GetExtent();
        renderPassInfo.clearValueCount = 1;
        vk::ClearValue clearColor = {};
        clearColor.color = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));
        renderPassInfo.pClearValues = &clearColor;

        commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

        vk::Viewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChain.GetExtent().width);
        viewport.height = static_cast<float>(swapChain.GetExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        commandBuffer.setViewport(0, viewport);
        vk::Rect2D scissor = {};
        scissor.offset = vk::Offset2D{ 0, 0 };
        scissor.extent = swapChain.GetExtent();
        commandBuffer.setScissor(0, scissor);

        // Descriptors
        vk::DescriptorSet descSets[] = { m_compositeImageDescriptorSet[imageIndex].get() };
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_compositePipeline.m_pipelineLayout, 0, descSets, {});

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_compositePipeline.m_pipeline.get());
        commandBuffer.draw(3, 1, 0, 0);
        commandBuffer.endRenderPass();
    }
    imageManager.GetSceneImage(imageIndex).TransformLayoutAsync(vk::ImageLayout::eColorAttachmentOptimal, commandBuffer);
    imageManager.GetBloom1Image(imageIndex).TransformLayoutAsync(vk::ImageLayout::eColorAttachmentOptimal, commandBuffer);
    imageManager.GetBloom2Image(imageIndex).TransformLayoutAsync(vk::ImageLayout::eColorAttachmentOptimal, commandBuffer);

    RFCT_VULKAN_CHECK(commandBuffer.end());
}

void bloomResurcesHolder::onSwapchainExtentChanged(RfctRenderImagesManager& imageManager, vk::Device device) {
    RFCT_PROFILE_FUNCTION();
    updateDescSets(imageManager, device);
}
