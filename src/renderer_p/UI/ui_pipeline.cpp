#include "renderer_p\renderer.h"
#include "ui_pipeline.h"


rfct::UIPipeline::UIPipeline() : m_vertexShader("shaders/UI/text_vert.spv"), m_fragShader("shaders/UI/text_frag.spv"), m_glyphsRenderData(RFCT_DEBUG_DRAW_VERTEX_BUFFER_MAX_SIZE, vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU), m_debugDrawglyphsRenderData(RFCT_DEBUG_DRAW_VERTEX_BUFFER_MAX_SIZE, vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU), m_defaultFont("fonts/jetbrainsMono-medium.txt")
{
    createPipeline();
    createDescriptorSet();
    /*GlyphVertex square[] = {
        // Bottom-left triangle
        { {-0.5f, -0.5f}, {0.0f, 0.0f} }, // Bottom-left
        { { 0.0f, -0.5f}, {1.0f, 0.0f} }, // Bottom-right
        { { 0.0f,  0.0f}, {1.0f, 1.0f} }, // Top-right

        // Top-right triangle
        { {-0.5f, -0.5f}, {0.0f, 0.0f} }, // Bottom-left
        { { 0.0f,  0.0f}, {1.0f, 1.0f} }, // Top-right
        { {-0.5f,  0.0f}, {0.0f, 1.0f} }  // Top-left
    };

    m_glyphsRenderData.buffer.CopyData((void*)square, sizeof(GlyphVertex)*6);
    m_glyphsRenderData.vertexCount = 6;*/
    std::string what = "I want to go home";
    
    addTextVertices(&m_glyphsRenderData,what, glm::vec2(0, 0), 0.3);
    std::string what1 = "hallo :3";
    addTextVertices(&m_glyphsRenderData,what1, glm::vec2(0, 25), 0.3);
}

rfct::UIPipeline::~UIPipeline()
{
}

void rfct::UIPipeline::createPipeline()
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
    auto bindingDescription = GlyphVertex::getBindingDescription();
    auto attributeDescriptions = GlyphVertex::getAttributeDescriptions();

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

    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
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
    pipelineLayoutInfo.setLayoutCount = 2;
    vk::DescriptorSetLayout dscSetLayouts[] = { cameraUbo::getDescriptorSetLayout(), getDescriptorSetLayout() };
    pipelineLayoutInfo.pSetLayouts = dscSetLayouts;
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
    pipelineInfo.renderPass = m_UIRenderPass.get();
    pipelineInfo.subpass = 0;

    m_pipeline = renderer::getRen().getDevice().createGraphicsPipelineUnique({}, pipelineInfo).value;

}

void rfct::UIPipeline::createRenderPass()
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

    m_UIRenderPass = renderer::getRen().getDevice().createRenderPassUnique(tempRenderPassInfo);
}

void rfct::UIPipeline::createDescriptorSet()
{

    vk::DescriptorPoolSize poolSize(
        vk::DescriptorType::eCombinedImageSampler,
        1
    );

    vk::DescriptorPoolCreateInfo poolCreateInfo(
        { vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet },
        1,
        1,
        &poolSize
    );

    m_DescriptorPool = renderer::getRen().getDevice().createDescriptorPoolUnique(poolCreateInfo);

    vk::DescriptorSetLayout dsLayout = getDescriptorSetLayout();
    vk::DescriptorSetAllocateInfo allocInfo(
        m_DescriptorPool.get(),
        1,
        &dsLayout
    );

    m_DescriptorSet = std::move(renderer::getRen().getDevice().allocateDescriptorSetsUnique(allocInfo)[0]);

    vk::DescriptorImageInfo imageInfo = {}; 
    imageInfo.imageView = m_defaultFont.m_TextureAtlas.m_Image.m_imageView;
    imageInfo.sampler = m_defaultFont.m_TextureAtlas.m_sampler.get();
    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::WriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.dstSet = m_DescriptorSet.get();
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pImageInfo = &imageInfo;

    renderer::getRen().getDevice().updateDescriptorSets({ writeDescriptorSet }, nullptr);
}




void rfct::UIPipeline::draw(frameData& fd, vk::Framebuffer framebuffer, uint32_t imageIndex)
{

    if (m_glyphsRenderData.vertexCount == 0 && m_debugDrawglyphsRenderData.vertexCount == 0)
    {
        return;
    }
    {
        RFCT_PROFILE_SCOPE("fences reset");
        RFCT_VULKAN_CHECK(renderer::getRen().getDevice().resetFences(1, &fd.m_uiInRenderFence.get()));
    }
    vk::CommandBuffer commandBuffer = fd.getuiCommandBuffer();
    {
        RFCT_PROFILE_SCOPE("begin command buffer");
        commandBuffer.reset({});
        vk::CommandBufferBeginInfo beginInfo = {};
        commandBuffer.begin(beginInfo);
    }



    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = m_UIRenderPass.get();
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


    // Descriptors
    vk::DescriptorSet descSets[] = { fd.getUICameraUboDescSet(), m_DescriptorSet.get() };
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout.get(), 0, descSets, {});

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());


    if (m_glyphsRenderData.vertexCount != 0) {
        vk::Buffer vertexBuffers[] = { m_glyphsRenderData.buffer.buffer };
        vk::DeviceSize offsets[] = { 0 };
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

        commandBuffer.draw(m_glyphsRenderData.vertexCount, 1, 0, 0);
    }
    if (m_debugDrawglyphsRenderData.vertexCount != 0) {
        vk::Buffer vertexBuffers[] = { m_debugDrawglyphsRenderData.buffer.buffer };
        vk::DeviceSize offsets[] = { 0 };
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

        commandBuffer.draw(m_debugDrawglyphsRenderData.vertexCount, 1, 0, 0);
    }
    commandBuffer.endRenderPass();

    {
        RFCT_PROFILE_SCOPE("submit command buffer");
        vk::CommandBuffer commandBuffer = fd.getuiCommandBuffer();
        commandBuffer.end();
        renderer::getRen().getDeviceWrapper().getQueueManager().submitGraphics(fd.uiSubmitInfo(), fd.getuiInRenderFence());
    }
    m_debugDrawglyphsRenderData.postFrame();
    {
        RFCT_PROFILE_SCOPE("fences wait");
        RFCT_VULKAN_CHECK(renderer::getRen().getDevice().waitForFences(1, &fd.m_uiInRenderFence.get(), VK_TRUE, UINT64_MAX));
    }


}

void rfct::UIPipeline::debugText(const std::string& text, glm::vec2 startPosition, float scale)
{
    addTextVertices(&m_debugDrawglyphsRenderData, text, startPosition, scale);
}

void rfct::UIPipeline::addTextVertices(glyphsRenderData* rd , const std::string& text, glm::vec2 position, float scale)
{
    vk::Extent2D windowExtent = renderer::getRen().getWindow().getExtent();

    float cursorX = position.x;
    float cursorY = position.y;
    std::vector<GlyphVertex> vertices;

    for (char c : text) {
        const glyph* g = m_defaultFont.getGlyph(c);

        float y0 = cursorY + g->yoffset * scale;
        float y1 = y0 + g->height * scale;

        float x0 = cursorX + g->xoffset * scale;
        float x1 = x0 + g->width * scale;

        size_t index = vertices.size();
        float atlasWidth = static_cast<float>(m_defaultFont.m_TextureAtlas.m_Image.width);
        float atlasHeight = static_cast<float>(m_defaultFont.m_TextureAtlas.m_Image.height);

        float u0 = g->x / atlasWidth;
        float v0 = g->y / atlasHeight;
        float u1 = (g->x + g->width) / atlasWidth;
        float v1 = (g->y + g->height) / atlasHeight;

        vertices.push_back({ {x0, y0}, {u0, v0} });
        vertices.push_back({ {x1, y0}, {u1, v0} });
        vertices.push_back({ {x1, y1}, {u1, v1} });
        vertices.push_back({ {x0, y1}, {u0, v1} });
        vertices.push_back(vertices[index]);
        vertices.push_back(vertices[index + 2]);

        cursorX += g->xadvance * scale;
    }

    rd->bufferOffset;
    char* mapped = (char*)rd->buffer.Map();
    mapped += rd->bufferOffset;
    memcpy(mapped, vertices.data(), vertices.size() * sizeof(vertices[0]));
    //rd->buffer.CopyData(vertices.data(), vertices.size() * sizeof(vertices[0]));

    rd->bufferOffset += vertices.size() * sizeof(vertices[0]);
    rd->buffer.Unmap();
    rd->vertexCount += vertices.size();
}

vk::DescriptorSetLayout rfct::UIPipeline::getDescriptorSetLayout()
{
    if (m_descriptorSetLayout) return m_descriptorSetLayout.get();
    vk::DescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.bindingCount = 1;
    layoutCreateInfo.pBindings = &layoutBinding;

    m_descriptorSetLayout = renderer::getRen().getDevice().createDescriptorSetLayoutUnique(layoutCreateInfo);

    return m_descriptorSetLayout.get();

}