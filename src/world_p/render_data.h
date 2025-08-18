#pragma once
#include "renderer_p\buffer\vulkan_buffer.h"
#include "renderer_p\buffer\vulkan_vertex_buffer.h"
#include "world_p\components.h"
#include "context.h"

namespace rfct {
	struct frameContext;
	class sceneRenderData {
	public:
		static vk::DescriptorSetLayout m_descriptorSetLayout;
		static vk::DescriptorSetLayout getDescriptorSetLayout();
		static void destroyDescriptorSetLayout();
	public:
		sceneRenderData();
		~sceneRenderData();
		void updateMat(const frameContext* ctx, const uint32_t& objIndexInSSBO, glm::mat4* mat);
		void updateDynamicVertices(const frameContext* ctx, const size_t objBufferOffset, void* vertices, const size_t size);
		uint32_t addStaticMat(void* data);
		uint32_t addDynamicMat(const frameContext* ctx, void* data);
		uint32_t addDynamicVertices(std::vector<Vertex>* vertices, uint32_t frame, uint32_t location = -1);
		objectLocation addStaticObject(std::vector<Vertex>* vertices, glm::mat4* matrix);
		objectLocation addDynamicObject(std::vector<Vertex>* vertices, glm::mat4* matrix, bool shouldAddToAllBuffers = true, const frameContext& fc = {});
		uint32_t reserveSuitableVertexBufferLocation(size_t numVertices);

		void removeDynamicObject(const dynamicSSBOIndexComponent& ssboData, const vertexRenderInfoComponent& vertexRenderInfo, bool addToFreelist = false, const frameContext* ctx = {});
		void removeDynamicEntity(entity e);


		vk::UniqueDescriptorPool m_DescriptorPool;

		// single buffer, bcs data shouldn't be changed after loadScene()
		vulkanVertexBuffer m_VertexBufferStatic;
		VulkanBuffer m_StaticModelMatsBuffer; // for model matrices of objects that are not changing frequently (ssbo)
		vk::UniqueDescriptorSet m_DescriptorSetStatic;
		void* m_mappedDataStatic;

		// separate buffers to avoid writting to a buffer while its data is still used by the GPU
		std::array<unique<VulkanBuffer>, RFCT_FRAMES_IN_FLIGHT> m_VertexBufferDynamic;
		std::array<VulkanBuffer, RFCT_FRAMES_IN_FLIGHT> m_DynamicModelMatsBuffers;
		std::array<vk::UniqueDescriptorSet, RFCT_FRAMES_IN_FLIGHT> m_DescriptorSetsDynamic;
		std::array<void*, RFCT_FRAMES_IN_FLIGHT> m_mappedMatsDataDynamic;
		std::array<void*, RFCT_FRAMES_IN_FLIGHT> m_mappedVerticesDataDynamic;

		size_t m_verticesCountStaticObj;
		size_t m_verticesCountDynamicObj;
	private:
		// should only be called in loadScene()
		inline void startTransferStatic() { m_mappedDataStatic = m_StaticModelMatsBuffer.Map(); };
		inline void endTransferStatic() { m_StaticModelMatsBuffer.Unmap(); };

		uint32_t m_matsCounterStatic;
		uint32_t m_matsCounterDynamic;

		// free lists for dynamic buffers
		std::vector<size_t> m_matricesFreeIndices;
		std::vector<vertexRenderInfoComponent> m_freeVertices;

		friend class scene;
	};
}