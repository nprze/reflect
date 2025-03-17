#include "debug_draw.h"
#include "sizes.h"
#include "renderer_p\renderer.h"
rfct::debugDraw* rfct::debugDraw::instance;

rfct::debugDraw::debugDraw() :m_triangleBuffer(RFCT_DEBUG_DRAW_VERTEX_BUFFER_MAX_SIZE, vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU), m_lineBuffer(RFCT_DEBUG_DRAW_VERTEX_BUFFER_MAX_SIZE, vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU), m_vertexShader("shaders/debug_draw/tri_vert.spv"), m_fragShader("shaders/debug_draw/tri_frag.spv")
{
    createPipelines();
	instance = this;
}


void rfct::debugDraw::createPipelines()
{
    createRenderPass();
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
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

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
        vk::DynamicState::eScissor,
        vk::DynamicState::eLineWidth
    };

    vk::PipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
    m_PipelineLayout = renderer::getRen().getDevice().createPipelineLayoutUnique(pipelineLayoutInfo);

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
    pipelineInfo.layout = m_PipelineLayout.get();
    pipelineInfo.renderPass = m_debugDrawRenderPass.get();
    pipelineInfo.subpass = 0;

    m_trianglePipeline = renderer::getRen().getDevice().createGraphicsPipelineUnique({}, pipelineInfo).value;

    // Line 
    vk::PipelineInputAssemblyStateCreateInfo lineinputAssembly = {};
    lineinputAssembly.topology = vk::PrimitiveTopology::eLineList;
    lineinputAssembly.primitiveRestartEnable = VK_FALSE;

    vk::GraphicsPipelineCreateInfo linePipelineInfo = {};
    linePipelineInfo.stageCount = 2;
    linePipelineInfo.pStages = shaderStages.data();
    linePipelineInfo.pVertexInputState = &vertexInputInfo;
    linePipelineInfo.pInputAssemblyState = &lineinputAssembly;
    linePipelineInfo.pRasterizationState = &rasterizer;
    linePipelineInfo.pMultisampleState = &multisampling;
    linePipelineInfo.pColorBlendState = &colorBlending;
    linePipelineInfo.pDepthStencilState = &depthStencil;
    linePipelineInfo.pDynamicState = &dynamicState;
    linePipelineInfo.pViewportState = &viewportState;
    linePipelineInfo.layout = m_PipelineLayout.get();
    linePipelineInfo.renderPass = m_debugDrawRenderPass.get();
    linePipelineInfo.subpass = 0;

    m_linePipeline = renderer::getRen().getDevice().createGraphicsPipelineUnique({}, linePipelineInfo).value;
}

void rfct::debugDraw::createRenderPass()
{
    vk::AttachmentDescription colorAttachment = {};
    colorAttachment.format = vk::Format::eB8G8R8A8Unorm;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::ePresentSrcKHR;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

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

    vk::RenderPassCreateInfo tempRenderPassInfo = {};
    tempRenderPassInfo.attachmentCount = 1;
    tempRenderPassInfo.pAttachments = &colorAttachment;
    tempRenderPassInfo.subpassCount = 1;
    tempRenderPassInfo.pSubpasses = &subpass;
    tempRenderPassInfo.dependencyCount = 2;
    tempRenderPassInfo.pDependencies = std::array{ dependency, dependency2 }.data();;

    m_debugDrawRenderPass = renderer::getRen().getDevice().createRenderPassUnique(tempRenderPassInfo);
}



rfct::debugDraw::~debugDraw()
{
}

void rfct::debugDraw::draw(frameData& fd, vk::Framebuffer framebuffer, uint32_t imageIndex)
{
    
	if (m_triangleBuffer.vertexCount == 0 && m_lineBuffer.vertexCount == 0)
	{
		return;
	}
    {
        RFCT_PROFILE_SCOPE("fences reset");
        RFCT_VULKAN_CHECK(renderer::getRen().getDevice().resetFences(1, &fd.m_debugDrawTrianglesInRenderFence.get()));
    }
    {
        RFCT_PROFILE_SCOPE("begin command buffer");
        vk::CommandBuffer commandBuffer = fd.getDebugDrawCommandBuffer();
        commandBuffer.reset({});
        vk::CommandBufferBeginInfo beginInfo = {};
        commandBuffer.begin(beginInfo);
    }

    vk::CommandBuffer commandBuffer = fd.getDebugDrawCommandBuffer();


    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = m_debugDrawRenderPass.get();
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    renderPassInfo.renderArea.extent = renderer::getRen().getDeviceWrapper().getSwapChain().getExtent();
    renderPassInfo.clearValueCount = 0;
    renderPassInfo.pClearValues = VK_NULL_HANDLE;

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    vk::Viewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(renderer::getRen().getDeviceWrapper().getSwapChain().getExtent().width);
    viewport.height = static_cast<float>(renderer::getRen().getDeviceWrapper().getSwapChain().getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    commandBuffer.setViewport(0, viewport);

    vk::Rect2D scissor = {};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = renderer::getRen().getDeviceWrapper().getSwapChain().getExtent();
    commandBuffer.setScissor(0, scissor);

    commandBuffer.setLineWidth(1.f);

    if(m_triangleBuffer.vertexCount!=0){
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_trianglePipeline.get());


        vk::Buffer vertexBuffers[] = { m_triangleBuffer.buffer.buffer };
        vk::DeviceSize offsets[] = { 0 };
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

        commandBuffer.draw(m_triangleBuffer.vertexCount, 1, 0, 0);
    }


    if (m_lineBuffer.vertexCount != 0) {

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_linePipeline.get());


        vk::Buffer vertexBuffersLine[] = { m_lineBuffer.buffer.buffer };
        vk::DeviceSize offsetsLine[] = { 0 };
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffersLine, offsetsLine);


        commandBuffer.draw(m_lineBuffer.vertexCount, 1, 0, 0);
    }
    commandBuffer.endRenderPass();

    {
        RFCT_PROFILE_SCOPE("submit command buffer");
        vk::CommandBuffer commandBuffer = fd.getDebugDrawCommandBuffer();
        commandBuffer.end();
        vk::CommandBuffer cmdbfr = fd.getDebugDrawCommandBuffer();
        renderer::getRen().getDeviceWrapper().getQueueManager().submitGraphics(fd.debugDrawSubmitInfo(), fd.getdebugDrawInRenderFence());
    }
	m_triangleBuffer.postFrame();
	m_lineBuffer.postFrame();
    {
        RFCT_PROFILE_SCOPE("fences wait");
        RFCT_VULKAN_CHECK(renderer::getRen().getDevice().waitForFences(1, &fd.m_debugDrawTrianglesInRenderFence.get(), VK_TRUE, UINT64_MAX));
    }

}

rfct::debugTriangle* rfct::debugDraw::requestNTriangles(uint32_t count)
{
    bool check = (m_triangleBuffer.vertexCount + 3*count) < RFCT_DEBUG_DRAW_VERTEX_BUFFER_MAX_SIZE;
	RFCT_ASSERT(check);
	void* ret = (char*)m_triangleBuffer.bufferMappedMemory + m_triangleBuffer.bufferOffset;
	m_triangleBuffer.vertexCount += count*3;
	m_triangleBuffer.bufferOffset += count * sizeof(debugTriangle);
	return (debugTriangle*)ret;
}

rfct::debugLine* rfct::debugDraw::requestNLines(uint32_t count)
{
    bool check = (m_lineBuffer.vertexCount + 2 * count) < RFCT_DEBUG_DRAW_VERTEX_BUFFER_MAX_SIZE;
    RFCT_ASSERT(check);
    void* ret = (char*)m_lineBuffer.bufferMappedMemory + m_lineBuffer.bufferOffset;
    m_lineBuffer.vertexCount += count * 3;
    m_lineBuffer.bufferOffset += count * sizeof(debugLine);
    return (debugLine*)ret;
}
