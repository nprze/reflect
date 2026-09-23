#include "render_objects.h"
#include "assets/assets_utils.h"
#include "assets/asset_manager.h"
#include "renderer_components.h"
#include <vma/vk_mem_alloc.h>
#include <fstream>

rfct::RfctShader::RfctShader(vk::Device device, const std::string& spirvFilePath) {
    RFCT_PROFILE_FUNCTION();
    std::ifstream file;
    if (!OpenAssetFile(spirvFilePath, &file, std::ios::binary | std::ios::ate)) {
        RFCT_CRITICAL("Failed to open shader file: {}", spirvFilePath);
    }
    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0);

    std::vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);
    file.close();

    vk::ShaderModuleCreateInfo createInfo = {};
    createInfo.setCodeSize(buffer.size());
    createInfo.setPCode(reinterpret_cast<const uint32_t*>(buffer.data()));

    m_shaderModule = device.createShaderModule(createInfo).value;
}

void rfct::RfctShader::DestroyShader(vk::Device device) {
	device.destroyShaderModule(m_shaderModule);
}

void rfct::RfctRenderBuffer::CreateBuffer(RfctRenderBufferSpec& spec, RfctVulkanMemAllocator* allocatorWrapper) {
    m_allocatorWrapperRef = allocatorWrapper;
    RFCT_PROFILE_FUNCTION();
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = spec.size;
    bufferCreateInfo.usage = static_cast<VkBufferUsageFlags>(spec.usage);
    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = spec.memoryUsage;
    allocCreateInfo.requiredFlags = spec.requiredFlags;
    allocCreateInfo.flags = spec.allocFlags | VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    allocCreateInfo.pUserData = (void*)spec.name;

    VkBuffer vkBuffer;
    VkResult res = vmaCreateBuffer(m_allocatorWrapperRef->GetAllocator(), &bufferCreateInfo,
        &allocCreateInfo, &vkBuffer, &m_allocation, nullptr);
    if (res != VK_SUCCESS) {
        RFCT_CRITICAL("Buffer creation failed with code: {0}", (uint32_t)res);
    }
    m_buffer = vk::Buffer(vkBuffer);
}

void rfct::RfctRenderBuffer::DestroyBuffer() {
    RFCT_PROFILE_FUNCTION();
    if (m_allocation) vmaDestroyBuffer(m_allocatorWrapperRef->GetAllocator(), static_cast<VkBuffer>(m_buffer), m_allocation);
}

void* rfct::RfctRenderBuffer::Map() {
    void* m_mappedData = nullptr;
    VkResult res = vmaMapMemory(m_allocatorWrapperRef->GetAllocator(), m_allocation, &m_mappedData);
    if (res != VK_SUCCESS) {
        RFCT_CRITICAL("Failed to map Vulkan buffer memory.");
    }
    return m_mappedData;
}

void rfct::RfctRenderBuffer::Unmap() {
    vmaUnmapMemory(m_allocatorWrapperRef->GetAllocator(), m_allocation);
}

void rfct::RfctRenderBuffer::CopyData(const void* data, size_t size) {
    RFCT_PROFILE_FUNCTION();
    void* m_mappedData = Map();
    std::memcpy(m_mappedData, data, size);
    Unmap();
}

rfct::RfctRenderPipeline::RfctRenderPipeline(const RfctRenderPipelineSpec& spec, vk::RenderPass renderPass, vk::Device device) {
	RFCT_PROFILE_FUNCTION();
	CreatePipeline(spec, renderPass, device);
}

void rfct::RfctRenderPipeline::CreatePipeline(const RfctRenderPipelineSpec& spec, vk::RenderPass renderPass, vk::Device device) {
	RFCT_PROFILE_FUNCTION();
	RfctShader* vshader = GetAssetManager().GetOrLoadShader(device, spec.vertexShaderPath);
	RfctShader* fshader = GetAssetManager().GetOrLoadShader(device, spec.fragmentShaderPath);
	// Shaders
	vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
	vertShaderStageInfo.module = vshader->getShaderModule();
	vertShaderStageInfo.pName = "main";

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
	fragShaderStageInfo.module = fshader->getShaderModule();
	fragShaderStageInfo.pName = "main";

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.vertexBindingDescriptionCount = spec.enableVetexBinding ? 1 : 0;
	vertexInputInfo.pVertexBindingDescriptions = spec.enableVetexBinding ? &spec.vertexInputBindingDescription : VK_NULL_HANDLE;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(spec.vertexInputAttributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = spec.vertexInputAttributeDescriptions.data();

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
	multisampling.rasterizationSamples = spec.MSAA4x ? vk::SampleCountFlagBits::e4 : vk::SampleCountFlagBits::e1;
	multisampling.sampleShadingEnable = VK_FALSE;

	// Color Blend State
	vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
    if (spec.enableColorBlend) {
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    }
    else {
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
    }
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
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(spec.descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = spec.descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(spec.pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = spec.pushConstantRanges.data();
	m_pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo).value;

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
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	m_graphicsPipeline = device.createGraphicsPipeline({}, pipelineInfo).value;
}

void rfct::RfctRenderPipeline::DestroyPipeline(vk::Device device) {
    device.destroyPipelineLayout(m_pipelineLayout);
    device.destroyPipeline(m_graphicsPipeline);
}

vk::DescriptorSetLayout uboDescriptorSetLayout;

vk::DescriptorSetLayout rfct::RfctUniformBuffer::GetUniformDescriptorSetLayout(vk::Device device) {
	if (uboDescriptorSetLayout) {
		return uboDescriptorSetLayout;
	}
	vk::DescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = 0;
	layoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
	layoutBinding.pImmutableSamplers = nullptr;

	vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.bindingCount = 1;
	layoutCreateInfo.pBindings = &layoutBinding;

	uboDescriptorSetLayout = device.createDescriptorSetLayout(layoutCreateInfo).value;
	return uboDescriptorSetLayout;
}

void rfct::RfctUniformBuffer::DestroyUniformDescriptorSetLayout(vk::Device device) {
	device.destroyDescriptorSetLayout(uboDescriptorSetLayout);
}

void rfct::RfctUniformBuffer::CreateUniformBuffer(RfctVulkanMemAllocator& memAllocatorWrapper, vk::Device device) {
    RfctRenderBuffer::RfctRenderBufferSpec bufferSpec;
    bufferSpec.name = "uniform buffer";
    bufferSpec.size = sizeof(RfctUniformData);
    bufferSpec.usage = vk::BufferUsageFlagBits::eUniformBuffer;
    bufferSpec.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    m_buffer.CreateBuffer(bufferSpec, &memAllocatorWrapper);

    m_mappedBuffer = m_buffer.Map();

    // Create pool
    std::array<vk::DescriptorPoolSize, 1> poolSizes = { {
       { vk::DescriptorType::eUniformBuffer, 1 },
    } };

    vk::DescriptorPoolCreateInfo poolCreateInfo(
        vk::DescriptorPoolCreateFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet), 1,
        poolSizes.size(), poolSizes.data()
    );
    m_descriptorPool = device.createDescriptorPoolUnique(poolCreateInfo).value;

    // Allocate uniform buffer descriptor set
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = m_descriptorPool.get();
    allocInfo.descriptorSetCount = 1;
    vk::DescriptorSetLayout descriptorSetLayout = RfctUniformBuffer::GetUniformDescriptorSetLayout(device);
    allocInfo.pSetLayouts = &descriptorSetLayout;
    auto descriptorSets = device.allocateDescriptorSetsUnique(allocInfo);
    m_cameraUboDescSet = std::move(descriptorSets.value[0]);

    // bind buffer to descriptor set
    if (!m_cameraUboDescSet.get()) {
        RFCT_CRITICAL("Camera UBO descriptor set is null");
    }
    vk::DescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_buffer.GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    vk::WriteDescriptorSet descriptorWrite{};
    descriptorWrite.dstSet = m_cameraUboDescSet.get();
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    device.updateDescriptorSets(descriptorWrite, nullptr);
}

void rfct::RfctUniformBuffer::DestroyUniformBuffer() {
	m_buffer.Unmap();
    m_buffer.DestroyBuffer();
}

void rfct::RfctUniformBuffer::UpdateUniformData(const RfctUniformData& newData) {
	RFCT_PROFILE_FUNCTION();
	memcpy(m_mappedBuffer, &newData, sizeof(RfctUniformData));
}

void rfct::RfctRenderImage::TransformLayoutSync(vk::ImageLayout newLayout, RfctDevice& deviceWrapper, RfctQueue& queue) {
    RFCT_PROFILE_FUNCTION();
    vk::CommandBufferAllocateInfo allocInfo(
        rfct::GetAssetsCommandPool(deviceWrapper),
        vk::CommandBufferLevel::ePrimary,
        1
    );
    auto cmdBuffersAllocResult = deviceWrapper.GetDevice().allocateCommandBuffers(allocInfo);
    RFCT_VULKAN_CHECK(cmdBuffersAllocResult.result);
    vk::CommandBuffer commandBuffer = cmdBuffersAllocResult.value[0];

    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    RFCT_VULKAN_CHECK(commandBuffer.begin(beginInfo));

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = m_currentLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    barrier.srcAccessMask = vk::AccessFlags{}; // oldLayout is always undefined

    if (newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    }
    else if (newLayout == vk::ImageLayout::ePresentSrcKHR) {
        barrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe;
    }
    else {
        RFCT_CRITICAL("Unsupported layout transition in transformImage");
    }

    commandBuffer.pipelineBarrier(
        sourceStage, destinationStage,
        vk::DependencyFlags{},
        nullptr, nullptr, barrier
    );

    RFCT_VULKAN_CHECK(commandBuffer.end());

    vk::SubmitInfo submitInfo({}, {}, commandBuffer);
    vk::FenceCreateInfo fenceInfo;
    auto fenceCreateResult = deviceWrapper.GetDevice().createFence(fenceInfo);
    RFCT_VULKAN_CHECK(fenceCreateResult.result);
    vk::Fence fence = fenceCreateResult.value;
    queue.SubmitGraphics(submitInfo, fence);
    RFCT_VULKAN_CHECK(deviceWrapper.GetDevice().waitForFences(fence, VK_TRUE, UINT64_MAX));

    m_currentLayout = newLayout;
    deviceWrapper.GetDevice().freeCommandBuffers(rfct::GetAssetsCommandPool(deviceWrapper), commandBuffer);
    deviceWrapper.GetDevice().destroyFence(fence);
}

void rfct::RfctRenderImage::TransformLayoutAsync(vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer) {
    RFCT_PROFILE_FUNCTION();
    vk::ImageSubresourceRange subresourceRange = {
        vk::ImageAspectFlagBits::eColor,
        0, 1,
        0, 1
    };
    vk::AccessFlags srcAccessMask;
    vk::AccessFlags dstAccessMask;
    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;

    if (m_currentLayout == vk::ImageLayout::eColorAttachmentOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        dstAccessMask = vk::AccessFlagBits::eShaderRead;
        srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (m_currentLayout == vk::ImageLayout::eShaderReadOnlyOptimal && newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        srcAccessMask = vk::AccessFlagBits::eShaderRead;
        dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eFragmentShader;
        dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    }
    else {
        RFCT_CRITICAL("Unsupported layout transition");
    }
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = m_currentLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange = subresourceRange;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;

    commandBuffer.pipelineBarrier(
        srcStage,
        dstStage,
        vk::DependencyFlags{},
        nullptr, nullptr,
        barrier
    );
    m_currentLayout = newLayout;
}

void rfct::RfctRenderImage::CreateImageAndView(const RfctRenderImageSpec& spec, RfctDevice& deviceWrapper, RfctVulkanInstance& instanceWrapper, RfctQueue& queueWrapper, RfctVulkanMemAllocator& allocatorWrapper) {
    RFCT_PROFILE_FUNCTION();
    m_format = spec.dafaultFormat;
    m_extent = spec.extent;
    m_debugName = spec.debugName;
    m_sampleCount = spec.imageSamples;
    if (spec.allocateImage) {
        AllocateImage(spec, allocatorWrapper);
    }
    else {
        m_image = spec.image;
    }
    TransformLayoutSync(spec.dafaultLayout, deviceWrapper, queueWrapper);
    instanceWrapper.SetObjectName(m_image, m_debugName, vk::ObjectType::eImage, deviceWrapper.GetDevice());
    CreateImageView(deviceWrapper.GetDevice());
}

void rfct::RfctRenderImage::InitFrameBuffer(std::vector<RfctRenderImage*> attachments, vk::RenderPass renderPass, vk::Device device) {
    RFCT_PROFILE_FUNCTION();
    std::vector<vk::ImageView> imageViews;
    imageViews.reserve(attachments.size());
    vk::Extent2D extent = m_extent;
    for (size_t i = 0; i < attachments.size(); i++) {
        RFCT_ASSERT(attachments[i] != nullptr);
        imageViews.push_back(attachments[i]->m_imageView.get());
        RFCT_ASSERT(attachments[i]->m_extent == extent && attachments[i]->m_extent != vk::Extent2D(1, 1));
    }
    vk::FramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.renderPass = renderPass;
    frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
    frameBufferCreateInfo.pAttachments = imageViews.data();
    frameBufferCreateInfo.width = extent.width;
    frameBufferCreateInfo.height = extent.height;
    frameBufferCreateInfo.layers = 1;
    m_frameBuffer = device.createFramebufferUnique(frameBufferCreateInfo).value;
    hasFrameBuffer = true;
}

void rfct::RfctRenderImage::Cleanup(RfctVulkanMemAllocator& allocatorWrapper, vk::Device device) {
    if (wasAllocatedUsingVMA) {
        vmaDestroyImage(allocatorWrapper.GetAllocator(), static_cast<VkImage>(m_image), m_imageAllocation);
        wasAllocatedUsingVMA = false;
    }
}

void rfct::RfctRenderImage::AllocateImage(const RfctRenderImage::RfctRenderImageSpec& spec, RfctVulkanMemAllocator& allocatorWrapper) {
    RFCT_PROFILE_FUNCTION();
    // Create Vulkan image
    vk::ImageCreateInfo imageInfo({}, vk::ImageType::e2D, m_format,
        { static_cast<uint32_t>(m_extent.width), static_cast<uint32_t>(m_extent.height), 1 }, 1, 1,
        spec.imageSamples, vk::ImageTiling::eOptimal,
        spec.usage,
        vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo imageAllocInfo{};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(allocatorWrapper.GetAllocator(), reinterpret_cast<const VkImageCreateInfo*>(&imageInfo), &imageAllocInfo,
        reinterpret_cast<VkImage*>(&m_image), &m_imageAllocation, nullptr) != VK_SUCCESS) {
        RFCT_CRITICAL("Failed to create Vulkan image");
    }
    wasAllocatedUsingVMA = true;
}

void rfct::RfctRenderImage::CreateImageView(vk::Device device) {
    RFCT_PROFILE_FUNCTION();
    vk::ImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.image = m_image;
    viewCreateInfo.viewType = vk::ImageViewType::e2D;
    viewCreateInfo.format = m_format;
    viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.layerCount = 1;
    auto imageViewResult = device.createImageViewUnique(viewCreateInfo);
    RFCT_VULKAN_CHECK(imageViewResult.result);
    m_imageView = std::move(imageViewResult.value);
}

void rfct::RfctRenderPass::CreateRenderPass(RfctRenderPassSpec& passSpec, vk::Device device) {
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
    if (passSpec.resolveAttachment) {
        subpass.pResolveAttachments = &resolveAttachmentRef;
    }

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

    vk::RenderPassCreateInfo renderPassInfo = {};
    std::array<vk::SubpassDependency, 2> dependencies = { dependency, dependency2 };
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    std::array<vk::AttachmentDescription, 2> attachments;
    attachments[0] = passSpec.colorAttachmentDesc;
    attachments[1] = passSpec.resolveAttachmentDesc;

    renderPassInfo.attachmentCount = static_cast<uint32_t>(passSpec.resolveAttachment ? 2 : 1);
    renderPassInfo.pAttachments = attachments.data();

    m_pass = device.createRenderPass(renderPassInfo).value;
}

void rfct::RfctRenderPass::DestroyRenderPass(vk::Device device) {
    device.destroyRenderPass(m_pass);
}
