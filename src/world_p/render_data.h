#pragma once
#include "renderer_p\buffer\vulkan_buffer.h"
#include "renderer_p\buffer\vulkan_vertex_buffer.h"
#include "world_p\components.h"
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
		inline void startTransferStatic() { m_mappedDataStatic = m_StaticModelMatsBuffer.Map(); };
		inline void endTransferStatic() { m_StaticModelMatsBuffer.Unmap(); };
		void updateMat(frameContext* ctx, const objectLocation& objLoc, glm::mat4* mat);
		uint32_t addStaticMat(void* data);
		uint32_t addDynamicMat(frameContext* ctx, void* data);
		objectLocation addStaticObject(std::vector<Vertex>* vertices, glm::mat4* matrix);
		objectLocation addDynamicObject(std::vector<Vertex>* vertices, glm::mat4* matrix);
		void preFrame();
		vulkanVertexBuffer m_VertexBufferStatic;
		VulkanBuffer m_StaticModelMatsBuffer; // for model matrices of object that are not changing frequently (ssbo)
		vk::UniqueDescriptorPool m_DescriptorPool;
		vk::UniqueDescriptorSet m_DescriptorSetStatic;

		std::array<unique<vulkanVertexBuffer>, RFCT_FRAMES_IN_FLIGHT> m_VertexBufferDynamic;
		std::array<VulkanBuffer, RFCT_FRAMES_IN_FLIGHT> m_DynamicModelMatsBuffers;
		std::array<vk::UniqueDescriptorSet, RFCT_FRAMES_IN_FLIGHT> m_DescriptorSetsDynamic;
		std::array<void*, RFCT_FRAMES_IN_FLIGHT> m_mappedDataDynamic;

		size_t m_verticesCountStaticObj;
		size_t m_verticesCountDynamicObj;
	private:
		void* m_mappedDataStatic;
		uint32_t m_matsCounterStatic;
		uint32_t m_matsCounterDynamic;
	};
}