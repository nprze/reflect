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
		inline void startTransfer() { m_mappedData = m_StaticModelMatsBuffer.Map(); };
		inline void endTransfer() { m_StaticModelMatsBuffer.Unmap(); };
		uint32_t addStaticMat(void* data);
		objectLocation addStaticObject(std::vector<Vertex>* vertices, glm::mat4* matrix);
		void preFrame();
		vulkanVertexBuffer m_VertexBuffer;
		VulkanBuffer m_StaticModelMatsBuffer; // for model matrices of object that are not changing frequently (ssbo)
		vk::UniqueDescriptorPool m_DescriptorPool;
		vk::UniqueDescriptorSet m_DescriptorSet;
		size_t m_verticesCount;
	private:
		void* m_mappedData;
		uint32_t m_matsCounter;
	};
}