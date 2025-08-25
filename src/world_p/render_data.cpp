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

rfct::sceneRenderData::sceneRenderData() : m_VertexBufferStatic(RFCT_SCENE_STATIC_DRAW_VERTEX_BUFFER_VERTEX_COUNT * sizeof(Vertex)), m_StaticModelMatsBuffer(sizeof(glm::mat4) * RFCT_MAX_STATIC_OBJ_ON_SCENE, vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU), m_mappedDataStatic(nullptr), m_matsCounterStatic(0), m_verticesCountStaticObj(0), m_verticesCountDynamicObj(0), m_matsCounterDynamic(0)
{
 	for (uint32_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; ++i) {
		m_VertexBufferDynamic[i] = std::make_unique<VulkanBuffer>(RFCT_SCENE_DYNAMIC_DRAW_VERTEX_BUFFER_VERTEX_COUNT *sizeof(Vertex), vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU);
		m_DynamicModelMatsBuffers[i] = std::move(VulkanBuffer(sizeof(glm::mat4) * RFCT_MAX_DYNAMIC_OBJ_ON_SCENE, vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU));
		m_mappedMatsDataDynamic[i] = nullptr;
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
			VK_WHOLE_SIZE
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
			m_DynamicModelMatsBuffers[i].buffer,
			0,
			VK_WHOLE_SIZE
		};


		vk::WriteDescriptorSet write{};
		write.dstSet = m_DescriptorSetsDynamic[i].get();
		write.dstBinding = 1;
		write.dstArrayElement = 0;
		write.descriptorType = vk::DescriptorType::eStorageBuffer;
		write.descriptorCount = 1;
		write.pBufferInfo = &bufferInfoDynamic;

		renderer::getRen().getDevice().updateDescriptorSets(1, &write, 0, nullptr);

		m_mappedMatsDataDynamic[i] = m_DynamicModelMatsBuffers[i].Map();
		m_mappedVerticesDataDynamic[i] = m_VertexBufferDynamic[i]->Map();
	}
}

rfct::sceneRenderData::~sceneRenderData()
{
	for (size_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; i++) {
		m_DynamicModelMatsBuffers[i].Unmap();
		m_VertexBufferDynamic[i]->Unmap();

	}
	destroyDescriptorSetLayout();
}

void rfct::sceneRenderData::updateMat(const frameContext* ctx, const uint32_t& objIndexInSSBO, glm::mat4* mat)
{
	char* finalPtr = (char*)m_mappedMatsDataDynamic[ctx->frame] + objIndexInSSBO * sizeof(glm::mat4);
	memcpy(finalPtr, mat, sizeof(glm::mat4));
}

void rfct::sceneRenderData::updateDynamicVertices(const frameContext* ctx, const size_t objBufferOffset, void* vertices, const size_t size)
{
	char* finalPtr = (char*)m_mappedVerticesDataDynamic[ctx->frame] + (objBufferOffset * sizeof(Vertex));
	memcpy(finalPtr, vertices, size);
}

uint32_t rfct::sceneRenderData::addStaticMat(void* data)
{
	if (!m_mappedDataStatic) { RFCT_CRITICAL("trying to add matrices when startTransferStatic() hasn't been called"); }
	char* finalPtr = ((char*)m_mappedDataStatic) + (m_matsCounterStatic * sizeof(glm::mat4));
	memcpy(finalPtr, data, sizeof(glm::mat4));
	return m_matsCounterStatic++;
}

uint32_t rfct::sceneRenderData::addDynamicMat(const frameContext* ctx, void* data)
{
	if (m_matricesFreeIndices.size() != 0) {
		size_t index = m_matricesFreeIndices.back();
		m_matricesFreeIndices.pop_back();
		char* finalPtr = ((char*)m_mappedMatsDataDynamic[ctx->frame]) + (index * sizeof(glm::mat4));
		memcpy(finalPtr, data, sizeof(glm::mat4));
		return index;

	}
	else {
		char* finalPtr = ((char*)m_mappedMatsDataDynamic[ctx->frame]) + (m_matsCounterDynamic * sizeof(glm::mat4));
		memcpy(finalPtr, data, sizeof(glm::mat4));
		RFCT_ASSERT(m_matsCounterDynamic < RFCT_MAX_DYNAMIC_OBJ_ON_SCENE);
		return m_matsCounterDynamic++;
	}
}

uint32_t rfct::sceneRenderData::reserveSuitableVertexBufferLocation(size_t numVertices) {
	
	uint32_t bestSizeDiff = INT_MAX;
	for (uint32_t i = 0; i < m_freeVertices.size(); ++i) {
		if (m_freeVertices[i].verticesCount == numVertices) {
			// found exact match
			uint32_t returnVal = m_freeVertices[i].vertexBufferOffset;
			m_freeVertices.erase(m_freeVertices.begin() + i);
			return returnVal;
		}
	}
	m_verticesCountDynamicObj += numVertices;
	RFCT_ASSERT(m_verticesCountDynamicObj < RFCT_SCENE_STATIC_DRAW_VERTEX_BUFFER_VERTEX_COUNT);
	return m_verticesCountDynamicObj - numVertices;
}

uint32_t rfct::sceneRenderData::addDynamicVertices(std::vector<Vertex>* vertices, uint32_t frame, uint32_t location)
{
	if (location == UINT32_MAX) {
		location = reserveSuitableVertexBufferLocation(vertices->size());
	}
	void* finalPtr = (char*)(m_mappedVerticesDataDynamic[frame]) + (location * sizeof(Vertex));
	std::memcpy(finalPtr, vertices->data(), vertices->size() * sizeof(Vertex));
	return location;
}

rfct::objectLocation rfct::sceneRenderData::addStaticObject(std::vector<Vertex>* vertices, glm::mat4* matrix)
{
	RFCT_PROFILE_FUNCTION();
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

rfct::objectLocation rfct::sceneRenderData::addDynamicObject(std::vector<Vertex>* vertices, glm::mat4* matrix, bool shouldAddToAllBuffers, const frameContext& fc)
{
	RFCT_PROFILE_FUNCTION();
	objectLocation objLoc{};
	if (shouldAddToAllBuffers) {
		frameContext ctx = {};
		objLoc.indexInSSBO = addDynamicMat(&ctx, matrix);
		for (uint8_t i = 1; i < RFCT_FRAMES_IN_FLIGHT; ++i) {
			ctx.frame = i;
			updateMat(&ctx, objLoc.indexInSSBO, matrix);
		}
	}
	else {
		objLoc.indexInSSBO = addDynamicMat(&fc, matrix);
		RFCT_CRITICAL("no.");
	}
	for (Vertex& ver : *vertices) {
		ver.objectIndex = objLoc.indexInSSBO;
	}

	if (shouldAddToAllBuffers) {
		objLoc.vertexBufferOffset = addDynamicVertices(vertices, 0);
		for (uint8_t i = 1; i < RFCT_FRAMES_IN_FLIGHT; ++i) {
			addDynamicVertices(vertices, i, objLoc.vertexBufferOffset);
		}
	}
	else {
		objLoc.vertexBufferOffset = addDynamicVertices(vertices, fc.frame);
		RFCT_CRITICAL("you know that the buffer offset is incremented each time you call it and the RFCT_FRAMES_IN_FLIGHT is not 1");
	}
	objLoc.verticesCount = vertices->size();
	m_verticesCountDynamicObj += objLoc.verticesCount;
 	return objLoc;
}

void rfct::sceneRenderData::removeDynamicObject(const dynamicSSBOIndexComponent& ssboData, const vertexRenderInfoComponent& vertexRenderInfo, bool addToFreelist, const frameContext* ctx)
{
	char* finalPtr = ((char*)m_mappedMatsDataDynamic[ctx->frame]) + (ssboData.indexInSSBO * sizeof(glm::mat4));
	memset(finalPtr, 0, sizeof(glm::mat4));
	char* finalPtrVer = ((char*)m_mappedVerticesDataDynamic[ctx->frame]) + (vertexRenderInfo.vertexBufferOffset * sizeof(Vertex));
	memset(finalPtrVer, 0, vertexRenderInfo.verticesCount * sizeof(Vertex));
	if (!addToFreelist) return;
	m_matricesFreeIndices.push_back(ssboData.indexInSSBO);
	m_freeVertices.push_back(vertexRenderInfo);
}

void rfct::sceneRenderData::removeDynamicEntity(entity e)
{
	const dynamicSSBOIndexComponent* ssbo = e.get<dynamicSSBOIndexComponent>();
	const vertexRenderInfoComponent* vData = e.get<vertexRenderInfoComponent>();
	frameContext noCtx = {};
	removeDynamicObject(*ssbo, *vData, true, &noCtx);
	for (uint8_t i = 1; i < RFCT_FRAMES_IN_FLIGHT; i++) {
		noCtx.frame = i;
		removeDynamicObject(*ssbo, *vData, false, &noCtx);
	}
}