#pragma once
#include "renderer_p\buffer\vulkan_buffer.h"
#include "renderer_p\buffer\vulkan_vertex_buffer.h"
#include "world_p\components.h"
namespace rfct {
	class sceneRenderData {
	public:
		static vk::DescriptorSetLayout m_descriptorSetLayout;
		static vk::DescriptorSetLayout getDescriptorSetLayout();
		static void destroyDescriptorSetLayout();
	public:
		sceneRenderData();
		~sceneRenderData();
		inline void startTransfer() { m_mappedDataStatic = m_StaticModelMatsBuffer.Map(); };
		inline void endTransfer() { m_StaticModelMatsBuffer.Unmap(); };
		void updateMat(const objectLocation& objLoc, glm::mat4* mat);
		uint32_t addStaticMat(void* data);
		uint32_t addDynamicMat(void* data);
		objectLocation addStaticObject(std::vector<Vertex>* vertices, glm::mat4* matrix);
		objectLocation addDynamicObject(std::vector<Vertex>* vertices, glm::mat4* matrix);
		void preFrame();
		vulkanVertexBuffer m_VertexBufferStatic;
		vulkanVertexBuffer m_VertexBufferDynamic;
		VulkanBuffer m_StaticModelMatsBuffer; // for model matrices of object that are not changing frequently (ssbo)
		VulkanBuffer m_DynamicModelMatsBuffer; // for model matrices of object that are changing frequently (ssbo) TODO: make it a dynamic ssbo
		vk::UniqueDescriptorPool m_DescriptorPool;
		vk::UniqueDescriptorSet m_DescriptorSetStatic;
		vk::UniqueDescriptorSet m_DescriptorSetDynamic;
		size_t m_verticesCountStaticObj;
		size_t m_verticesCountDynamicObj;
	private:
		void* m_mappedDataStatic;
		void* m_mappedDataDynamic;
		uint32_t m_matsCounterStatic;
		uint32_t m_matsCounterDynamic;
	};
}