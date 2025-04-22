#include "vulkan_rasterizer_pipeline.h"

#include "renderer_p/renderer.h"
#include "vertex.h"
#include "renderer_p/frame/frame_data.h"
#include "world_p/render_data.h"
#include "world_p/scene.h"


rfct::vulkanRasterizerPipeline::vulkanRasterizerPipeline() :m_vertexShader("shaders/cube/cube_vert.spv"), m_fragShader("shaders/cube/cube_frag.spv")
{
    createRenderPass();
	createPipeline();
}

rfct::vulkanRasterizerPipeline::~vulkanRasterizerPipeline()
{
}

void rfct::vulkanRasterizerPipeline::createPipeline()
{
    // Shaders
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = m_vertexShader.getShaderModule();
    vertShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = m_fragShader.getShaderModule();
    fragShaderStageInfo.pName = "main";

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

    // Input
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Rasterization State
    vk::PipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eNone;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Multisample State
    vk::PipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.rasterizationSamples = msaaSamples;
    multisampling.sampleShadingEnable = VK_FALSE;

    // Color Blend State
    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
    colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
    colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
    colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA;

    vk::PipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    vk::PipelineDepthStencilStateCreateInfo depthStencil = {};

    // Dynamic State
    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();


    // Pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.setLayoutCount = 2;
    vk::DescriptorSetLayout dscSetLayouts[] = { cameraUbo::getDescriptorSetLayout(), sceneRenderData::getDescriptorSetLayout() };
    pipelineLayoutInfo.pSetLayouts = dscSetLayouts;
    m_pipelineLayout = renderer::getRen().getDevice().createPipelineLayoutUnique(pipelineLayoutInfo);

    vk::PipelineViewportStateCreateInfo viewportState = {};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Pipeline
    vk::GraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.layout = m_pipelineLayout.get();
    pipelineInfo.renderPass = m_renderPass.get();
    pipelineInfo.subpass = 0;

    m_graphicsPipeline = renderer::getRen().getDevice().createGraphicsPipelineUnique({}, pipelineInfo).value;
}


void rfct::vulkanRasterizerPipeline::createRenderPass()
{
    vk::AttachmentDescription colorAttachment = {};
    colorAttachment.format = vk::Format::eB8G8R8A8Unorm;
    colorAttachment.samples = msaaSamples;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;


    vk::AttachmentDescription resolveAttachment = {};
    resolveAttachment.format = vk::Format::eB8G8R8A8Unorm;
    resolveAttachment.samples = vk::SampleCountFlagBits::e1;
    resolveAttachment.loadOp = vk::AttachmentLoadOp::eDontCare;
    resolveAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    resolveAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    resolveAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    resolveAttachment.initialLayout = vk::ImageLayout::eUndefined;
    resolveAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference resolveAttachmentRef = {};
    resolveAttachmentRef.attachment = 1;
    resolveAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pResolveAttachments = &resolveAttachmentRef;

    vk::SubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.srcAccessMask = vk::AccessFlagBits::eNone;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

    vk::SubpassDependency dependency2 = {};
    dependency2.srcSubpass = 0;
    dependency2.dstSubpass = VK_SUBPASS_EXTERNAL;
    dependency2.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency2.dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    dependency2.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    dependency2.dstAccessMask = vk::AccessFlagBits::eNone;
    dependency2.dependencyFlags = vk::DependencyFlagBits::eByRegion;

    std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, resolveAttachment };

    vk::RenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    std::array<vk::SubpassDependency, 2> dependencies = { dependency, dependency2 };
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    m_renderPass = renderer::getRen().getDevice().createRenderPassUnique(renderPassInfo);

}

void rfct::vulkanRasterizerPipeline::recordCommandBuffer(frameContext* ctx, frameData& frameData, vk::Framebuffer framebuffer)
{
    RFCT_PROFILE_FUNCTION();
    const sceneRenderData& renderdata = ctx->scene->getRenderData();
    vk::CommandBuffer commandBuffer = frameData.m_sceneCommandBuffer.get();

    commandBuffer.reset({});
    vk::CommandBufferBeginInfo beginInfo = {};
    commandBuffer.begin(beginInfo);

    std::array<vk::ClearValue, 1> clearValues = {};
    clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});

    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = m_renderPass.get();
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    renderPassInfo.renderArea.extent = renderer::getRen().getDeviceWrapper().getSwapChain().getExtent();
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline.get());
    


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

        vk::DescriptorSet sets[] = { frameData.getCameraUboDescSet(ctx->frame), renderdata.m_DescriptorSetStatic.get() };
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0, sets, {});

        commandBuffer.draw(renderdata.m_verticesCountStaticObj, 1, 0, 0);
    }
    if (renderdata.m_verticesCountDynamicObj) {

        vk::Buffer vertexBuffers[] = { renderdata.m_VertexBufferDynamic[ctx->frame]->m_Buffer.buffer};
        
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

        vk::DescriptorSet sets[] = { frameData.getCameraUboDescSet(ctx->frame), renderdata.m_DescriptorSetsDynamic[ctx->frame].get() };
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0, sets, {});

        commandBuffer.draw(renderdata.m_verticesCountDynamicObj, 1, 0, 0);
    }

    commandBuffer.endRenderPass();
    commandBuffer.end();

}
