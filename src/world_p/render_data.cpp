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

rfct::sceneRenderData::sceneRenderData() : m_VertexBufferStatic(RFCT_DEBUG_DRAW_VERTEX_BUFFER_MAX_SIZE), m_StaticModelMatsBuffer(sizeof(glm::mat4)* RFCT_MAX_STATIC_OBJ_ON_SCENE, vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU), m_mappedDataStatic(nullptr), m_matsCounterStatic(0), m_verticesCountStaticObj(0), m_verticesCountDynamicObj(0), m_matsCounterDynamic(0)
{
 	for (uint32_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; ++i) {
		m_VertexBufferDynamic[i] = std::make_unique<vulkanVertexBuffer>(RFCT_DEBUG_DRAW_VERTEX_BUFFER_MAX_SIZE);
		m_DynamicModelMatsBuffers[i] = std::make_unique<VulkanBuffer>(sizeof(glm::mat4) * 20, vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU);
		m_mappedDataDynamic[i] = nullptr;
	}
	m_verticesCountDynamicObj = 0;
	m_matsCounterDynamic = 0;

	std::array<vk::DescriptorPoolSize, 1> poolSizes = { {
		{ vk::DescriptorType::eStorageBuffer, 1 + RFCT_FRAMES_IN_FLIGHT }
	} };

	vk::DescriptorPoolCreateInfo poolCreateInfo(
		vk::DescriptorPoolCreateFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet),
		1 + RFCT_FRAMES_IN_FLIGHT,
		poolSizes.size(),
		poolSizes.data()
	);
	m_DescriptorPool = renderer::getRen().getDevice().createDescriptorPoolUnique(poolCreateInfo);

	{
		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.descriptorPool = m_DescriptorPool.get();
		allocInfo.descriptorSetCount = 1;
		vk::DescriptorSetLayout descriptorSetLayout = getDescriptorSetLayout();
		allocInfo.pSetLayouts = &descriptorSetLayout;

		auto descriptorSets = renderer::getRen().getDevice().allocateDescriptorSetsUnique(allocInfo);
		m_DescriptorSetStatic = std::move(descriptorSets[0]);

		vk::DescriptorBufferInfo bufferInfoStatic = {
			m_StaticModelMatsBuffer.buffer,
			0,
			sizeof(glm::mat4) * 20
		};

		vk::WriteDescriptorSet write{};
		write.dstSet = m_DescriptorSetStatic.get();
		write.dstBinding = 1;
		write.dstArrayElement = 0;
		write.descriptorType = vk::DescriptorType::eStorageBuffer;
		write.descriptorCount = 1;
		write.pBufferInfo = &bufferInfoStatic;

		renderer::getRen().getDevice().updateDescriptorSets(1, &write, 0, nullptr);
	}

	// Allocate dynamic descriptor sets
	for (uint32_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; ++i) {
		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.descriptorPool = m_DescriptorPool.get();
		allocInfo.descriptorSetCount = 1;
		vk::DescriptorSetLayout descriptorSetLayout = getDescriptorSetLayout();
		allocInfo.pSetLayouts = &descriptorSetLayout;

		auto descriptorSets = renderer::getRen().getDevice().allocateDescriptorSetsUnique(allocInfo);
		m_DescriptorSetsDynamic[i] = std::move(descriptorSets[0]);

		vk::DescriptorBufferInfo bufferInfoDynamic = {
			m_DynamicModelMatsBuffers[i]->buffer,
			0,
			sizeof(glm::mat4) * 20
		};


		vk::WriteDescriptorSet write{};
		write.dstSet = m_DescriptorSetsDynamic[i].get();
		write.dstBinding = 1;
		write.dstArrayElement = 0;
		write.descriptorType = vk::DescriptorType::eStorageBuffer;
		write.descriptorCount = 1;
		write.pBufferInfo = &bufferInfoDynamic;

		renderer::getRen().getDevice().updateDescriptorSets(1, &write, 0, nullptr);

		// Map buffer immediately if desired
		m_mappedDataDynamic[i] = m_DynamicModelMatsBuffers[i]->Map();
	}
}

rfct::sceneRenderData::~sceneRenderData()
{
	for (size_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; i++) {
		m_DynamicModelMatsBuffers[i]->Unmap();
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
	uint32_t matLocation = addDynamicMat(&noCtx, matrix);
	objLoc.indexInSSBO = matLocation;
	for (Vertex& ver : *vertices) {
		ver.objectIndex = matLocation;
	}
	objLoc.verticesCount = vertices->size();
	m_verticesCountDynamicObj += objLoc.verticesCount;
	for (size_t i = 0;i<RFCT_FRAMES_IN_FLIGHT;i++)
		objLoc.vertexBufferOffset = m_VertexBufferDynamic[i]->copyData(*vertices);
	return objLoc;
}

void rfct::sceneRenderData::preFrame()
{

}
