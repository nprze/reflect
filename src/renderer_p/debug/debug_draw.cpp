#include "debug_draw.h"
#include "sizes.h"
#include "renderer_p\renderer.h"
rfct::debugDraw* rfct::debugDraw::instance;

rfct::debugDraw::debugDraw() :m_Buffer(RFCT_DEBUG_DRAW_VERTEX_BUFFER_MAX_TRIANGLE_COUNT * sizeof(debugTriangle), vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU), m_triangleVertexShader("shaders/debug_draw/tri_vert.spv"), m_triangleFragShader("shaders/debug_draw/tri_frag.spv"),
m_Offset(0), m_TriangleCount(0)
{
    createTrianglePipeline();
	m_MappedMemory = m_Buffer.Map();
	instance = this;
}


void rfct::debugDraw::createTrianglePipeline()
{
    createTriangleRenderPass();
    // Shaders
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = m_triangleVertexShader.getShaderModule();
    vertShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = m_triangleFragShader.getShaderModule();
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
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
    m_trianglePipelineLayout = renderer::getRen().getDevice().createPipelineLayoutUnique(pipelineLayoutInfo);

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
    pipelineInfo.layout = m_trianglePipelineLayout.get();
    pipelineInfo.renderPass = m_triangleRenderPass.get();
    pipelineInfo.subpass = 0;

    m_trianglePipeline = renderer::getRen().getDevice().createGraphicsPipelineUnique({}, pipelineInfo).value;
}

void rfct::debugDraw::createTriangleRenderPass()
{
    vk::AttachmentDescription colorAttachment = {};
    colorAttachment.format = vk::Format::eB8G8R8A8Unorm;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
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

    m_triangleRenderPass = renderer::getRen().getDevice().createRenderPassUnique(tempRenderPassInfo);
}



rfct::debugDraw::~debugDraw()
{
	m_Buffer.Unmap();
}

void rfct::debugDraw::draw(frameData& fd, vk::Framebuffer framebuffer, uint32_t imageIndex)
{
    
	if (m_TriangleCount == 0)
	{
		return;
	}
    
    {
        RFCT_PROFILE_SCOPE("begin command buffer");
        vk::CommandBuffer commandBuffer = fd.getDebugTrianglesCommandBuffer();
        commandBuffer.reset({});
        vk::CommandBufferBeginInfo beginInfo = {};
        commandBuffer.begin(beginInfo);
    }

    vk::CommandBuffer commandBuffer = fd.getDebugTrianglesCommandBuffer();


    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = m_triangleRenderPass.get();
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    renderPassInfo.renderArea.extent = renderer::getRen().getDeviceWrapper().getSwapChain().getExtent();
    renderPassInfo.clearValueCount = 0;
    renderPassInfo.pClearValues = VK_NULL_HANDLE;

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_trianglePipeline.get());


    vk::Buffer vertexBuffers[] = { m_Buffer.buffer };
    vk::DeviceSize offsets[] = { 0 };
    commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

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

    commandBuffer.draw(m_TriangleCount*3, 1, 0, 0);
    commandBuffer.endRenderPass();

    {
        RFCT_PROFILE_SCOPE("submit command buffer");
        vk::CommandBuffer commandBuffer = fd.getDebugTrianglesCommandBuffer();
        commandBuffer.end();
        vk::CommandBuffer cmdbfr = fd.getDebugTrianglesCommandBuffer();
        renderer::getRen().getDeviceWrapper().getQueueManager().submitGraphics(fd.debugDrawSubmitInfo(), fd.getdebugDrawInRenderFence());
    }
	m_TriangleCount = 0;
	m_Offset = 0;

}

rfct::debugTriangle* rfct::debugDraw::requestNTriangles(uint32_t count)
{
    bool check = (m_TriangleCount + count) < RFCT_DEBUG_DRAW_VERTEX_BUFFER_MAX_TRIANGLE_COUNT;
	RFCT_ASSERT(check);
	void* ret = (char*)m_MappedMemory + m_Offset;
	m_TriangleCount += count;
	m_Offset += count * sizeof(debugTriangle);
	return (debugTriangle*)ret;
}
