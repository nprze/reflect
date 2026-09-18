#pragma once
#include <vulkan/vulkan.hpp>
#include <string>
#include "renderer_p/buffer/vulkan_buffer.h"
#include <glm/glm.hpp>

namespace rfct {
    class RfctShader {
    public:
        vk::ShaderModule getShaderModule() { return m_shaderModule; }
    public:
        RfctShader(vk::Device device, const std::string& spirvFilePath);
		void DestroyShader(vk::Device device);
    private:
        vk::ShaderModule m_shaderModule;
    };

	class RfctRenderPipeline {
	public:
		struct RfctRenderPipelineSpec {
			std::string vertexShaderPath;
			std::string fragmentShaderPath;
			vk::VertexInputBindingDescription vertexInputBindingDescription;
			std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
			std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
			bool MSAA4x = false;
		};
	public:
		RfctRenderPipeline(const RfctRenderPipelineSpec& spec, vk::RenderPass renderPass, vk::Device device);
		void CreatePipeline(const RfctRenderPipelineSpec& spec, vk::RenderPass renderPass, vk::Device device);
		void DestroyPipeline(vk::Device device);
		vk::Pipeline GetPipeline() { return m_graphicsPipeline; };
	private:
		vk::PipelineLayout m_pipelineLayout;
		vk::Pipeline m_graphicsPipeline;
	};

	struct RfctUniformData {
		glm::mat4 vp;
		float globalTime;
		float changeSceneEffectMultiplier;
	};

	class RfctUniformBuffer {
	public:
		static vk::DescriptorSetLayout GetUniformDescriptorSetLayout(vk::Device device);
		static void DestroyUniformDescriptorSetLayout(vk::Device device);
	public:
		vk::DescriptorSet& GetCameraDescSet() { return m_cameraUboDescSet.get(); }
		vk::Buffer GetBuffer() { return m_buffer.buffer; }
	public:
		RfctUniformBuffer(vk::Device device);
		void DestroyUniformBuffer();
		void UpdateUniformData(const RfctUniformData& newData);
	private:
		void BindBufferToDescriptor(vk::Buffer buffer, vk::Device device);
	private:
		VulkanBuffer m_buffer;
		void* m_mappedBuffer;
		vk::UniqueDescriptorPool m_descriptorPool;
		vk::UniqueDescriptorSet m_cameraUboDescSet;
	};

	class RfctRenderImage {
	public:
		struct RfctRenderImageSpec {
			// settings
			vk::Extent2D extent = { 1, 1 };
			vk::Format dafaultFormat = vk::Format::eB8G8R8A8Unorm;
			std::string debugName = "renderImage";
			// image create
			bool allocateImage = true;
			vk::Image image = nullptr; // should hold valid image handle if allocateImage is false
			VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
			vk::SampleCountFlagBits imageSamples = vk::SampleCountFlagBits::e1;
			vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
			// runtime settings
			vk::ImageLayout dafaultLayout = vk::ImageLayout::eColorAttachmentOptimal;
		};
	public:
		void TransformLayoutSync(vk::ImageLayout newLayout, RfctDevice& deviceWrapper, RfctQueue& queue);
		void TransformLayoutAsync(vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer);
		void CreateImageAndView(const RfctRenderImageSpec& spec, RfctDevice& deviceWrapper,
			RfctVulkanInstance& instanceWrapper, RfctQueue& queueWrapper, RfctVulkanMemAllocator& allocatorWrapper);
		void InitFrameBuffer(std::vector<RfctRenderImage*> attachments, vk::RenderPass renderPass, vk::Device device);
		void Cleanup(RfctVulkanMemAllocator& allocatorWrapper, vk::Device device);
	private:
		void AllocateImage(const RfctRenderImage::RfctRenderImageSpec& spec, RfctDevice& deviceWrapper,
			RfctQueue& queueWrapper, RfctVulkanMemAllocator& allocatorWrapper);
		void CreateImageView(vk::Device device);
	public:
		bool hasFrameBuffer;
		bool wasAllocatedUsingVMA = false; // usually yes, swap chain could be one exception
		vk::Extent2D m_extent;
		std::string m_debugName;
		vk::Image m_image;
		VmaAllocation m_imageAllocation;
		vk::UniqueImageView m_imageView;
		vk::UniqueFramebuffer m_frameBuffer;
		vk::ImageLayout m_currentLayout = vk::ImageLayout::eUndefined;
		vk::Format m_format;
		vk::SampleCountFlags m_sampleCount;
	};
}