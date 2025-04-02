#include "render_data.h"
#include "renderer_p\rasterizer_pipeline\vertex.h"
#include "renderer_p\renderer.h"

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

rfct::sceneRenderData::sceneRenderData() : m_VertexBuffer(RFCT_DEBUG_DRAW_VERTEX_BUFFER_MAX_SIZE), m_StaticModelMatsBuffer(sizeof(glm::mat4) * 20, vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU), m_mappedData(nullptr), m_matsCounter(0), m_verticesCount(0)
{
	// Descriptor Pool
	std::array<vk::DescriptorPoolSize, 1> poolSizes = { {
	   { vk::DescriptorType::eStorageBuffer, 1 }
   } };

	vk::DescriptorPoolCreateInfo poolCreateInfo(
		vk::DescriptorPoolCreateFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet),
		1,
		poolSizes.size(),
		poolSizes.data()
	);
	m_DescriptorPool = renderer::getRen().getDevice().createDescriptorPoolUnique(poolCreateInfo);
	



	// Descriptor Set
	vk::DescriptorSetAllocateInfo allocInfo{};
	allocInfo.descriptorPool = m_DescriptorPool.get();
	allocInfo.descriptorSetCount = 1;
	vk::DescriptorSetLayout descriptorSetLayout = getDescriptorSetLayout();
	allocInfo.pSetLayouts = &descriptorSetLayout;

	std::vector<vk::UniqueDescriptorSet> descriptorSets = renderer::getRen().getDevice().allocateDescriptorSetsUnique(allocInfo);
	m_DescriptorSet = std::move(descriptorSets[0]);

    vk::DescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = m_StaticModelMatsBuffer.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(glm::mat4) * 20;

    vk::WriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.dstSet = m_DescriptorSet.get();
    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eStorageBuffer;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pBufferInfo = &bufferInfo;

	renderer::getRen().getDevice().updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
}

rfct::sceneRenderData::~sceneRenderData()
{
	destroyDescriptorSetLayout();
}

uint32_t rfct::sceneRenderData::addStaticMat(void* data)
{
	if (!m_mappedData) { RFCT_CRITICAL("trying to add matrices when startTransfer() hasn't been called"); }
	char* finalPtr = ((char*)m_mappedData) + (m_matsCounter * sizeof(glm::mat4));
	memcpy(finalPtr, data, sizeof(glm::mat4));
	return m_matsCounter++;
}

rfct::objectLocation rfct::sceneRenderData::addStaticObject(std::vector<Vertex>* vertices, glm::mat4* matrix)
{
	objectLocation objLoc{};
	objLoc.SSBO = &m_StaticModelMatsBuffer;
	uint32_t matLocation = addStaticMat(matrix);
	objLoc.indexInSSBO = matLocation;
	for (Vertex& ver : *vertices) {
		ver.objectIndex = matLocation;
	}
	m_VertexBuffer.copyData(*vertices);
	m_verticesCount += (*vertices).size();
	return objLoc;
}

void rfct::sceneRenderData::preFrame()
{

}
