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

		std::vector<vulkanVertexBuffer> m_VertexBufferDynamic;
		std::vector<vk::UniqueDescriptorSet> m_DescriptorSetsDynamic;
		std::vector<VulkanBuffer> m_DynamicModelMatsBuffer; // for model matrices of object that are changing frequently (ssbo) TODO: make it a dynamic ssbo

		size_t m_verticesCountStaticObj;
		size_t m_verticesCountDynamicObj;
	private:
		void* m_mappedDataStatic;
		std::vector<void*> m_mappedDataDynamic;
		uint32_t m_matsCounterStatic;
		uint32_t m_matsCounterDynamic;
	};
}