#include "render_objects.h"
#include "assets/assets_utils.h"
#include "assets/asset_manager.h"
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

    m_shaderModule = device.createShaderModuleUnique(createInfo).value;
}

rfct::RfctRenderPipeline::RfctRenderPipeline(const RfctRenderPipelineSpec& spec, vk::RenderPass renderPass, vk::Device device)
	: m_vertexShader(GetAssetManager().GetOrLoadShader(device, spec.vertexShaderPath)),
	m_fragShader(GetAssetManager().GetOrLoadShader(device, spec.fragmentShaderPath)) {
	RFCT_PROFILE_FUNCTION();
	CreatePipeline(spec, renderPass, device);
}

void rfct::RfctRenderPipeline::CreatePipeline(const RfctRenderPipelineSpec& spec, vk::RenderPass renderPass, vk::Device device) {
	RFCT_PROFILE_FUNCTION();
	// Shaders
	vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
	vertShaderStageInfo.module = m_vertexShader->getShaderModule();
	vertShaderStageInfo.pName = "main";

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
	fragShaderStageInfo.module = m_fragShader->getShaderModule();
	fragShaderStageInfo.pName = "main";

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &spec.vertexInputBindingDescription;
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
	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e4;
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
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(spec.descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = spec.descriptorSetLayouts.data();
	m_pipelineLayout = device.createPipelineLayoutUnique(pipelineLayoutInfo).value;

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
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	m_graphicsPipeline = device.createGraphicsPipelineUnique({}, pipelineInfo).value;
}