#include "ui_pipeline.h"
#include "renderer_p/renderer.h"


rfct::UIPipelines::UIPipelines(vk::RenderPass renderPass)
    : m_vertexMostShader("shaders/UI/UIeverything_vert.spv"), 
    m_fragMostShader("shaders/UI/UIeverything_frag.spv"), 
    m_vertexImageShader("shaders/UI/UIimage_vert.spv"),
    m_fragImageShader("shaders/UI/UIimage_frag.spv"),
    m_imageVertexBuffer(6 * RFCT_MAX_BUTTON_COUNT * sizeof(GlyphVertex)),
    m_UIVertexBuffer(RFCT_MAX_UI_CHARS * 6 * sizeof(GlyphVertex)),
    m_debugDrawUIVertexBuffer(RFCT_DEBUG_DRAW_VERTEX_BUFFER_MAX_SIZE),
    m_defaultFont("fonts/3MTrislan.txt"),
    m_dummyImage(""),
	m_emptyImage("UI/empty.png") {
    createPipeline(renderPass);
    createDescriptorSet();
}

void rfct::UIPipelines::createPipeline(vk::RenderPass renderPass)  {
	RFCT_PROFILE_FUNCTION();
    // Shaders
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = m_vertexMostShader.getShaderModule();
    vertShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = m_fragMostShader.getShaderModule();
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
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.setLayoutCount = 2;
    vk::DescriptorSetLayout dscSetLayouts[] = { ubo::getDescriptorSetLayout(), getDescriptorSetLayout() };
    pipelineLayoutInfo.pSetLayouts = dscSetLayouts;
    m_PipelineLayout = renderer::getRen().getDevice().createPipelineLayoutUnique(pipelineLayoutInfo);


    vk::PipelineViewportStateCreateInfo viewportState = {};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Pipeline for most things
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
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    m_pipeline = renderer::getRen().getDevice().createGraphicsPipelineUnique({}, pipelineInfo).value;

    // Image Pipeline (for dialogues)
    vk::PipelineShaderStageCreateInfo newvertShaderStageInfo = {};
    newvertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    newvertShaderStageInfo.module = m_vertexImageShader.getShaderModule();
    newvertShaderStageInfo.pName = "main";


    vk::PipelineShaderStageCreateInfo newfragShaderStageInfo = {};
    newfragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    newfragShaderStageInfo.module = m_fragImageShader.getShaderModule();
    newfragShaderStageInfo.pName = "main";

    std::vector<vk::PipelineShaderStageCreateInfo> newshaderStages = { newvertShaderStageInfo, newfragShaderStageInfo };

    pipelineInfo.pStages = newshaderStages.data();

    m_imagePipeline = renderer::getRen().getDevice().createGraphicsPipelineUnique({}, pipelineInfo).value;
}

void rfct::UIPipelines::createDescriptorSet() {
    RFCT_PROFILE_FUNCTION();
    vk::DescriptorPoolSize poolSize(
        vk::DescriptorType::eCombinedImageSampler,
        RFCT_UI_TEXTURE_BINDINGS
    );

    vk::DescriptorPoolCreateInfo poolCreateInfo(
        { vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet},
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
	m_textureIndexMap.reserve(RFCT_UI_TEXTURE_BINDINGS);
}

void rfct::UIPipelines::draw(frameData& fd, vk::Framebuffer framebuffer, vk::RenderPass renderPass) {
    RFCT_PROFILE_FUNCTION();
    if (m_UIVertexBuffer.vertexCount == 0 && m_debugDrawUIVertexBuffer.vertexCount == 0) return;

    vk::CommandBuffer commandBuffer = fd.m_uiCommandBuffer.get();

    commandBuffer.reset({});
    vk::CommandBufferBeginInfo beginInfo = {};
    commandBuffer.begin(beginInfo);

    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    renderPassInfo.renderArea.extent = rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent();
    renderPassInfo.clearValueCount = 0;
    renderPassInfo.pClearValues = VK_NULL_HANDLE;

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    vk::Viewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().width);
    viewport.height = static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    commandBuffer.setViewport(0, viewport);

    vk::Rect2D scissor = {};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent();
    commandBuffer.setScissor(0, scissor);


    // Descriptors
    vk::DescriptorSet descSets[] = { fd.getUICameraUboDescSet(), m_DescriptorSet.get() };
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout.get(), 0, descSets, {});
    
    if (m_imageVertexBuffer.vertexCount != 0) {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_imagePipeline.get());

        vk::Buffer vertexBuffers[] = { m_imageVertexBuffer.buffer.buffer };
        vk::DeviceSize offsets[] = { 0 };
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

        commandBuffer.draw(m_imageVertexBuffer.vertexCount, 1, 0, 0);
    }
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());
    if (m_debugDrawUIVertexBuffer.vertexCount != 0) {
        vk::Buffer vertexBuffers[] = { m_debugDrawUIVertexBuffer.buffer.buffer };
        vk::DeviceSize offsets[] = { 0 };
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

        commandBuffer.draw(m_debugDrawUIVertexBuffer.vertexCount, 1, 0, 0);
    }
    // order is important for dialogue- text should be drawn last
    if (m_UIVertexBuffer.vertexCount != 0) {
        vk::Buffer vertexBuffers[] = { m_UIVertexBuffer.buffer.buffer };
        vk::DeviceSize offsets[] = { 0 };
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

        commandBuffer.draw(m_UIVertexBuffer.vertexCount, 1, 0, 0);
    }

    commandBuffer.endRenderPass();

    commandBuffer.end();
    m_imageVertexBuffer.postFrame();
    m_UIVertexBuffer.postFrame();
    m_debugDrawUIVertexBuffer.postFrame();
}

float rfct::UIPipelines::debugText(const std::string& text, glm::vec2 startPosition, float scale) {
    return addTextVertices(&m_debugDrawUIVertexBuffer, text, startPosition, scale);
}

void rfct::UIPipelines::beginAddingTriangles() {
    m_BufferMappedMemory = (char*)m_UIVertexBuffer.buffer.Map();

    widthFactor = static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().width);
    heightFactor = static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().height);
}

void rfct::UIPipelines::addTriangleNormalized(const glm::vec2& vec0, const glm::vec2& vec1, const glm::vec2& vec2, const glm::vec3& color, opacity op) {
    GlyphVertex vertices[3];
    vertices[0].pos = glm::vec2{vec0.x * widthFactor, vec0.y * heightFactor};
    vertices[1].pos = glm::vec2{vec1.x * widthFactor, vec1.y * heightFactor};
    vertices[2].pos = glm::vec2{vec2.x * widthFactor, vec2.y * heightFactor};

    switch (op) {
    case opacity25percent: {
        vertices[0].texCoord = { 0, 0 };
        vertices[1].texCoord = { 0.4, 0 };
        vertices[2].texCoord = { 0, 0.4 };
        break;
    }
    case opacity50percent: {
        vertices[0].texCoord = { 0.6, 0 };
        vertices[1].texCoord = { 1.0, 0 };
        vertices[2].texCoord = { 0.6, 0.4 };
        break;
    }
    case opacity75percent: {
        vertices[0].texCoord = { 0, 0.6 };
        vertices[1].texCoord = { 0.4, 0.6 };
        vertices[2].texCoord = { 0, 1.0 };
        break;
    }
    case opacity100percent: {
        vertices[0].texCoord = { 0.6, 0.6 };
        vertices[1].texCoord = { 1.0, 0.6 };
        vertices[2].texCoord = { 0.6, 1.0 };
        break;
    }
    }

    int texIndex = getTextureIndex(&m_emptyImage, imageUsage::ui);

    vertices[0].texIndex = texIndex;
    vertices[1].texIndex = texIndex;
    vertices[2].texIndex = texIndex;

    vertices[0].color = color;
    vertices[1].color = color;
    vertices[2].color = color;

    memcpy(m_BufferMappedMemory + m_UIVertexBuffer.bufferOffset, vertices, 3 * sizeof(vertices[0]));

    m_UIVertexBuffer.bufferOffset += 3 * sizeof(vertices[0]);
    m_UIVertexBuffer.vertexCount += 3;
}

void rfct::UIPipelines::endAddingTriangles() {
    m_UIVertexBuffer.buffer.Unmap();
}

int rfct::UIPipelines::getTextureIndex(bindableImage* image, imageUsage usage) {
    if (m_textureIndexMap.find(image) != m_textureIndexMap.end()) {
        return m_textureIndexMap[image];
    }
    // basic checks
	RFCT_ASSERT(!(usage == imageUsage::fontAtlas && m_indexTextureMap[0] != nullptr)); // attempt to get to font atlas when it is already set
	if (m_textureIndexMap.size() >= RFCT_UI_TEXTURE_BINDINGS) {
		RFCT_CRITICAL("Attempt to add texture when the descriptor has been filled (max textures: {})", RFCT_UI_TEXTURE_BINDINGS);
	}

    // get the actual index based on usage
    int indexInShader;
    if (usage == imageUsage::fontAtlas) {
		indexInShader = 0;
	}
	else {
		if (m_textureIndexMap.size() == 0) {
			indexInShader = 1;
		}
		else {
			indexInShader = m_textureIndexMap.size();
		}
	}
	m_textureIndexMap[image] = indexInShader;
	m_indexTextureMap[indexInShader] = image;

    // update images
    vk::DescriptorImageInfo imageInfo[RFCT_UI_TEXTURE_BINDINGS];

    for (int i = 0; i < RFCT_UI_TEXTURE_BINDINGS; ++i) {
        auto it = m_indexTextureMap.find(i);
        if (it != m_indexTextureMap.end()) {
            imageInfo[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            imageInfo[i].imageView = it->second->m_Image.m_imageView;
            imageInfo[i].sampler = it->second->m_sampler.get();
        }
        else {
            imageInfo[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            imageInfo[i].imageView = m_dummyImage.m_Image.m_imageView;
            imageInfo[i].sampler = m_dummyImage.m_sampler.get();
        }
    }

    vk::WriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.dstSet = m_DescriptorSet.get();
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    writeDescriptorSet.descriptorCount = RFCT_UI_TEXTURE_BINDINGS;
    writeDescriptorSet.pImageInfo = imageInfo;
    
    renderer::getRen().getDevice().updateDescriptorSets({ writeDescriptorSet }, nullptr);

    return indexInShader;
}

void rfct::UIPipelines::addImage(const glm::vec2& min, const glm::vec2& max, bindableImage* image, const glm::vec2& texCoordmin, const glm::vec2& texCoordmax) {
    RFCT_PROFILE_FUNCTION();
	GlyphVertex vertices[6];
	vertices[0].pos = { min.x, min.y };
	vertices[1].pos = { max.x, min.y };
	vertices[2].pos = { max.x, max.y };
	vertices[3].pos = { min.x, max.y };
	vertices[4].pos = { min.x, min.y };
	vertices[5].pos = { max.x, max.y };

	vertices[0].texCoord = texCoordmin;
	vertices[1].texCoord = { texCoordmax.x, texCoordmin.y };
	vertices[2].texCoord = texCoordmax;
	vertices[3].texCoord = { texCoordmin.x, texCoordmax.y };
	vertices[4].texCoord = texCoordmin;
	vertices[5].texCoord = texCoordmax;

	int texIndex = getTextureIndex(image, imageUsage::ui);
    RFCT_ASSERT(texIndex > 0);
	
    vertices[0].texIndex = texIndex;
	vertices[1].texIndex = texIndex;
	vertices[2].texIndex = texIndex;
	vertices[3].texIndex = texIndex;
	vertices[4].texIndex = texIndex;
    vertices[5].texIndex = texIndex;
    
	vertices[0].color = { 1.f, 1.f, 1.f };
	vertices[1].color = { 1.f, 1.f, 1.f };
	vertices[2].color = { 1.f, 1.f, 1.f };
	vertices[3].color = { 1.f, 1.f, 1.f };
	vertices[4].color = { 1.f, 1.f, 1.f };
	vertices[5].color = { 1.f, 1.f, 1.f };

    char* mapped = (char*)m_imageVertexBuffer.buffer.Map();
    mapped += m_imageVertexBuffer.bufferOffset;
    memcpy(mapped, vertices, 6 * sizeof(GlyphVertex));

    m_imageVertexBuffer.bufferOffset += 6 * sizeof(GlyphVertex);
    m_imageVertexBuffer.buffer.Unmap();
    m_imageVertexBuffer.vertexCount += 6;
}

void rfct::UIPipelines::removeImage(bindableImage* image) {
    RFCT_PROFILE_FUNCTION();
	int texIndex = getTextureIndex(image, imageUsage::ui);
    for (auto it = m_textureIndexMap.begin(); it != m_textureIndexMap.end(); ) {
        if (it->second == texIndex) {
            it = m_textureIndexMap.erase(it); 
        }
        else {
            ++it;
        }
    }
    for (auto it = m_indexTextureMap.begin(); it != m_indexTextureMap.end(); ) {
        if (it->second == image) {
            it = m_indexTextureMap.erase(it);
        }
        else {
            ++it;
        }
    }
}

float rfct::UIPipelines::addTextVertices(UIVertexBuffer* rd, const std::string& text, glm::vec2 position, float scale, const glm::vec3& color, font* f) {
    RFCT_PROFILE_FUNCTION();
    if (!f) f = &m_defaultFont;
    vk::Extent2D windowExtent = renderer::getRen().getWindow().getExtent();
    
	int textureIndexInShader = getTextureIndex(&f->m_TextureAtlas, imageUsage::fontAtlas);

    float cursorX = position.x;
    float cursorY = position.y;
    std::vector<GlyphVertex> vertices;

    for (char c : text) {
        const glyph* g = f->getGlyph(c);

        float y0 = cursorY + g->yoffset * scale;
        float y1 = y0 + g->height * scale;

        float x0 = cursorX + g->xoffset * scale;
        float x1 = x0 + g->width * scale;

        size_t index = vertices.size();
        float atlasWidth = static_cast<float>(f->m_TextureAtlas.m_Image.width);
        float atlasHeight = static_cast<float>(f->m_TextureAtlas.m_Image.height);

        float u0 = g->x / atlasWidth;
        float v0 = g->y / atlasHeight;
        float u1 = (g->x + g->width) / atlasWidth;
        float v1 = (g->y + g->height) / atlasHeight;

        vertices.push_back({ {x0, y0}, {u0, v0}, color, textureIndexInShader });
        vertices.push_back({ {x1, y0}, {u1, v0}, color, textureIndexInShader });
        vertices.push_back({ {x1, y1}, {u1, v1}, color, textureIndexInShader });
        vertices.push_back({ {x0, y1}, {u0, v1}, color, textureIndexInShader });
        vertices.push_back(vertices[index]);
        vertices.push_back(vertices[index + 2]);

        cursorX += g->xadvance * scale;
    }

    char* mapped = (char*)rd->bufferMappedMemory;
    mapped += rd->bufferOffset;
    memcpy(mapped, vertices.data(), vertices.size() * sizeof(vertices[0]));

    rd->bufferOffset += vertices.size() * sizeof(vertices[0]);
    rd->vertexCount += vertices.size();

    return cursorX;
}

float rfct::UIPipelines::addTextVerticesHeight(const std::string& text, glm::vec2 position, float height, const glm::vec3& color, font* f) {
    RFCT_PROFILE_FUNCTION();
    if (!f) f = &m_defaultFont;
    float scale = f->fontScale * height ;
    return addTextVertices(&m_UIVertexBuffer, text, position, scale, color, f);
}

vk::DescriptorSetLayout rfct::UIPipelines::getDescriptorSetLayout() {
    if (m_descriptorSetLayout) return m_descriptorSetLayout.get();
    RFCT_PROFILE_FUNCTION();

    vk::DescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    layoutBinding.descriptorCount = RFCT_UI_TEXTURE_BINDINGS;
    layoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.bindingCount = 1;
    layoutCreateInfo.pBindings = &layoutBinding;

    m_descriptorSetLayout = renderer::getRen().getDevice().createDescriptorSetLayoutUnique(layoutCreateInfo);

    return m_descriptorSetLayout.get();

}