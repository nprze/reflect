#include "render_data.h"
#include "renderer_p\rasterizer_pipeline\vertex.h"
#include "renderer_p\renderer.h"
#include "context.h"

vk::DescriptorSetLayout rfct::sceneRenderData::m_descriptorSetLayout;

vk::DescriptorSetLayout rfct::sceneRenderData::getDescriptorSetLayout()
{
	if (m_descriptorSetLayout) return m_descriptorSetLayout;
	vk::DescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = 1;
	layoutBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
	layoutBinding.pImmutableSamplers = nullptr;

	vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.bindingCount = 1;
	layoutCreateInfo.pBindings = &layoutBinding;

	m_descriptorSetLayout = renderer::getRen().getDevice().createDescriptorSetLayout(layoutCreateInfo);
	return m_descriptorSetLayout;
}

void rfct::sceneRenderData::destroyDescriptorSetLayout()
{
	renderer::getRen().getDevice().destroyDescriptorSetLayout(m_descriptorSetLayout);
}

rfct::sceneRenderData::sceneRenderData() : m_VertexBufferStatic(RFCT_DEBUG_DRAW_VERTEX_BUFFER_MAX_SIZE), m_StaticModelMatsBuffer(sizeof(glm::mat4)* RFCT_MAX_STATIC_OBJ_ON_SCENE, vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU), m_mappedDataStatic(nullptr), m_matsCounterStatic(0), m_verticesCountStaticObj(0), m_verticesCountDynamicObj(0), m_matsCounterDynamic(0), m_DynamicModelMatsBuffer(RFCT_FRAMES_IN_FLIGHT, VulkanBuffer(sizeof(glm::mat4)* RFCT_MAX_STATIC_OBJ_ON_SCENE, vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU)), m_VertexBufferDynamic (RFCT_FRAMES_IN_FLIGHT, vulkanVertexBuffer(RFCT_DEBUG_DRAW_VERTEX_BUFFER_MAX_SIZE))
{

	m_DescriptorSetsDynamic.reserve(RFCT_FRAMES_IN_FLIGHT);
	m_mappedDataDynamic.reserve(RFCT_FRAMES_IN_FLIGHT);
	// Descriptor Pool
	constexpr uint32_t maxSets = RFCT_FRAMES_IN_FLIGHT + 1;
	std::array<vk::DescriptorPoolSize, 1> poolSizes = { {
	   { vk::DescriptorType::eStorageBuffer, maxSets }
	} };

	vk::DescriptorPoolCreateInfo poolCreateInfo(
		vk::DescriptorPoolCreateFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet),
		maxSets,
		poolSizes.size(),
		poolSizes.data()
	);
	m_DescriptorPool = renderer::getRen().getDevice().createDescriptorPoolUnique(poolCreateInfo);

	// Descriptor Set
	{
		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.descriptorPool = m_DescriptorPool.get();
		allocInfo.descriptorSetCount = 1;
		vk::DescriptorSetLayout descriptorSetLayout = getDescriptorSetLayout();
		allocInfo.pSetLayouts = &descriptorSetLayout;

		std::vector<vk::UniqueDescriptorSet> descriptorSets = renderer::getRen().getDevice().allocateDescriptorSetsUnique(allocInfo);
		m_DescriptorSetStatic = std::move(descriptorSets[0]);
	}
	for (size_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; i++) {
		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.descriptorPool = m_DescriptorPool.get();
		allocInfo.descriptorSetCount = 1;
		vk::DescriptorSetLayout descriptorSetLayout = getDescriptorSetLayout();
		allocInfo.pSetLayouts = &descriptorSetLayout;

		std::vector<vk::UniqueDescriptorSet> descriptorSets = renderer::getRen().getDevice().allocateDescriptorSetsUnique(allocInfo);
		
			m_DescriptorSetsDynamic.emplace_back(std::move(descriptorSets[0]));
	}
	{
		vk::DescriptorBufferInfo bufferInfoStatic = {};
		bufferInfoStatic.buffer = m_StaticModelMatsBuffer.buffer;
		bufferInfoStatic.offset = 0;
		bufferInfoStatic.range = sizeof(glm::mat4) * RFCT_MAX_STATIC_OBJ_ON_SCENE;


		vk::WriteDescriptorSet writeDescriptorSets[1];
		writeDescriptorSets[0].dstSet = m_DescriptorSetStatic.get();
		writeDescriptorSets[0].dstBinding = 1;
		writeDescriptorSets[0].dstArrayElement = 0;
		writeDescriptorSets[0].descriptorType = vk::DescriptorType::eStorageBuffer;
		writeDescriptorSets[0].descriptorCount = 1;
		writeDescriptorSets[0].pBufferInfo = &bufferInfoStatic;
		renderer::getRen().getDevice().updateDescriptorSets(1, writeDescriptorSets, 0, nullptr);
	}
	for (size_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; i++){

		vk::DescriptorBufferInfo bufferInfoDynamic = {};
		bufferInfoDynamic.buffer = m_DynamicModelMatsBuffer[i].buffer;
		bufferInfoDynamic.offset = 0;
		bufferInfoDynamic.range = sizeof(glm::mat4) * RFCT_MAX_STATIC_OBJ_ON_SCENE;


		vk::WriteDescriptorSet writeDescriptorSets[1];
		writeDescriptorSets[0].dstSet = m_DescriptorSetsDynamic[i].get();
		writeDescriptorSets[0].dstBinding = 1;
		writeDescriptorSets[0].dstArrayElement = 0;
		writeDescriptorSets[0].descriptorType = vk::DescriptorType::eStorageBuffer;
		writeDescriptorSets[0].descriptorCount = 1;
		writeDescriptorSets[0].pBufferInfo = &bufferInfoDynamic;
		renderer::getRen().getDevice().updateDescriptorSets(1, writeDescriptorSets, 0, nullptr);
		m_mappedDataDynamic.emplace_back(m_DynamicModelMatsBuffer[i].Map());
	}

}

rfct::sceneRenderData::~sceneRenderData()
{
	for (size_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; i++) {
		 m_DynamicModelMatsBuffer[i].Unmap();
	}
	destroyDescriptorSetLayout();
}

void rfct::sceneRenderData::updateMat(frameContext* ctx, const objectLocation& objLoc, glm::mat4* mat)
{
	char* finalPtr = (char*)m_mappedDataDynamic[ctx->frame] + objLoc.indexInSSBO * sizeof(glm::mat4);
	memcpy(finalPtr, mat, sizeof(glm::mat4));
}

uint32_t rfct::sceneRenderData::addStaticMat(void* data)
{
	if (!m_mappedDataStatic) { RFCT_CRITICAL("trying to add matrices when startTransferStatic() hasn't been called"); }
	char* finalPtr = ((char*)m_mappedDataStatic) + (m_matsCounterStatic * sizeof(glm::mat4));
	memcpy(finalPtr, data, sizeof(glm::mat4));
	return m_matsCounterStatic++;
}
uint32_t rfct::sceneRenderData::addDynamicMat(frameContext* ctx, void* data)
{
	char* finalPtr = ((char*)m_mappedDataDynamic[ctx->frame]) + (m_matsCounterDynamic * sizeof(glm::mat4));
	memcpy(finalPtr, data, sizeof(glm::mat4));
	return m_matsCounterDynamic++;
}

rfct::objectLocation rfct::sceneRenderData::addStaticObject(std::vector<Vertex>* vertices, glm::mat4* matrix)
{
	objectLocation objLoc{};
	uint32_t matLocation = addStaticMat(matrix);
	objLoc.indexInSSBO = matLocation;
	for (Vertex& ver : *vertices) {
		ver.objectIndex = matLocation;
	}
	objLoc.verticesCount = vertices->size();
	m_verticesCountStaticObj += objLoc.verticesCount;
	objLoc.vertexBufferOffset = m_VertexBufferStatic.copyData(*vertices);
	return objLoc;
}

rfct::objectLocation rfct::sceneRenderData::addDynamicObject(std::vector<Vertex>* vertices, glm::mat4* matrix)
{
	objectLocation objLoc{};
	frameContext noCtx{};
	uint32_t matLocation = addDynamicMat({}, matrix);
	objLoc.indexInSSBO = matLocation;
	for (Vertex& ver : *vertices) {
		ver.objectIndex = matLocation;
	}
	objLoc.verticesCount = vertices->size();
	m_verticesCountDynamicObj += objLoc.verticesCount;
	objLoc.vertexBufferOffset = m_VertexBufferDynamic[noCtx.frame].copyData(*vertices);
	return objLoc;
}

void rfct::sceneRenderData::preFrame()
{

}
