#pragma once
#include <vulkan/vulkan.hpp>
#include <string>
#include <glm/glm.hpp>
#include <vma/vk_mem_alloc.h>

namespace rfct {
	class RfctDevice;
	class RfctVulkanInstance;
	class RfctQueue;
	class RfctVulkanMemAllocator;

    class RfctShader {
    public:
        vk::ShaderModule getShaderModule() { return m_shaderModule; }
    public:
        RfctShader(vk::Device device, const std::string& spirvFilePath);
		void DestroyShader(vk::Device device);
    private:
        vk::ShaderModule m_shaderModule;
    };

	class RfctRenderBuffer {
	public:
		struct RfctRenderBufferSpec {
			const char* name;
			vk::DeviceSize size;
			vk::BufferUsageFlags usage;
			VmaMemoryUsage memoryUsage;
			VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			VmaAllocationCreateFlags allocFlags = 0;
		};
	public:
		vk::Buffer GetBuffer() { return m_buffer; }
		void CreateBuffer(RfctRenderBufferSpec& spec, RfctVulkanMemAllocator* allocatorWrapper);
		void DestroyBuffer();
		void* Map();
		void Unmap();
		void CopyData(const void* data, size_t size);
	private:
		RfctVulkanMemAllocator* m_allocatorWrapperRef = nullptr;
		vk::Buffer m_buffer;
		VmaAllocation m_allocation = nullptr;
	};

	class RfctRenderPipeline {
	public:
		struct RfctRenderPipelineSpec {
			std::string vertexShaderPath;
			std::string fragmentShaderPath;
			vk::VertexInputBindingDescription vertexInputBindingDescription;
			std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
			std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
			std::vector<vk::PushConstantRange> pushConstantRanges;
			bool MSAA4x = false;
			bool enableVetexBinding = true;
			bool enableColorBlend = false;
		};
	public:
		RfctRenderPipeline(const RfctRenderPipelineSpec& spec, vk::RenderPass renderPass, vk::Device device);
		void CreatePipeline(const RfctRenderPipelineSpec& spec, vk::RenderPass renderPass, vk::Device device);
		void DestroyPipeline(vk::Device device);
		vk::Pipeline GetPipeline() { return m_graphicsPipeline; };
		vk::PipelineLayout GetPipelineLayout() { return m_pipelineLayout; };
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
		vk::Buffer GetBuffer() { return m_buffer.GetBuffer(); }
	public:
		void CreateUniformBuffer(RfctVulkanMemAllocator& memAllocatorWrapper, vk::Device device);
		void DestroyUniformBuffer();
		void UpdateUniformData(const RfctUniformData& newData);
	private:
		RfctRenderBuffer m_buffer;
		void* m_mappedBuffer;
		vk::UniqueDescriptorPool m_descriptorPool;
		vk::UniqueDescriptorSet m_cameraUboDescSet;
	};

	class RfctRenderPass {
	public:
		struct RfctRenderPassSpec {
			vk::AttachmentDescription colorAttachmentDesc;
			bool resolveAttachment = false;
			vk::AttachmentDescription resolveAttachmentDesc;
		};
	public:
		vk::RenderPass GetPass() { return m_pass; }
		void CreateRenderPass(RfctRenderPassSpec& passSpec, vk::Device device);
		void DestroyRenderPass(vk::Device device);
	private:
		vk::RenderPass m_pass;
		vk::ImageLayout m_prePassFramebufferLayout;
		vk::ImageLayout m_postPassFramebufferLayout;
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
		void AllocateImage(const RfctRenderImage::RfctRenderImageSpec& spec, RfctVulkanMemAllocator& allocatorWrapper);
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