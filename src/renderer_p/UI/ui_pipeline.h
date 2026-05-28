#pragma once
#include "font/font.h"
#include "renderer_p/shader/vulkan_shader.h"
#include "renderer_p/frame/frame_data.h"

namespace rfct {
	struct UIVertexBuffer {
		inline UIVertexBuffer(uint32_t size, const std::string& debugName = "glyphsVertexBuffer")
			: buffer(debugName.c_str(), size, vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU),
			bufferOffset(0), vertexCount(0) {
 			bufferMappedMemory = buffer.Map();
		};
		inline ~UIVertexBuffer() { 
			buffer.Unmap(); 
		};
		inline void postFrame() { bufferOffset = 0; vertexCount = 0; };
		VulkanBuffer buffer;
		uint32_t bufferOffset;
		uint32_t vertexCount;
		void* bufferMappedMemory;
	};
	enum opacity {
		opacity25percent,
		opacity50percent,
		opacity75percent,
		opacity100percent
	};
	class UIPipelines {
	public:
		UIPipelines(vk::RenderPass renderPass);
		void createPipeline(vk::RenderPass renderPass);
		void createDescriptorSet();
		void draw(frameData& fd, vk::Framebuffer framebuffer, vk::RenderPass renderPass);
		float debugText(const std::string& text, glm::vec2 startPosition, float scale);

		float addTextVertices(UIVertexBuffer* rd, const std::string& text, glm::vec2 position, float scale, const glm::vec3& color = { 1.f, 0.f, 0.f }, font* f = nullptr); // returns the cursor end x position
		float addTextVerticesHeight(const std::string& text, glm::vec2 position, float height, const glm::vec3& color = { 1.f, 0.f, 0.f }, font* f = nullptr); // takes in height (in 0.0 to  1.0)
		inline float addTextVertices(const std::string& text, glm::vec2 position, float scale, const glm::vec3& color = { 1.f, 0.f, 0.f }, font* f = nullptr) {
			return addTextVertices(&m_UIVertexBuffer, text, position, scale, color, f);
		}

		void beginAddingTriangles();
		void addTriangleNormalized(const glm::vec2& vec0, const glm::vec2& vec1, const glm::vec2& vec2, const glm::vec3& color, opacity op);
		void endAddingTriangles();
		int getTextureIndex(bindableImage* image, imageUsage usage);
		void addImage(const glm::vec2& min, const glm::vec2& max, bindableImage* image, const glm::vec2& texCoordMin = { 0.f,0.f }, const glm::vec2& texCoordMax = { 1.f,1.f });
		void removeImage(bindableImage* image);

		font* getDefaultFont() { return &m_defaultFont; }
		vk::DescriptorSetLayout getDescriptorSetLayout();
	private:
		vulkanShader m_vertexMostShader;
		vulkanShader m_fragMostShader;
		vulkanShader m_vertexImageShader;
		vulkanShader m_fragImageShader;
		vk::UniquePipelineLayout m_PipelineLayout;
		vk::UniqueDescriptorSetLayout m_descriptorSetLayout;
		vk::UniqueDescriptorPool m_DescriptorPool;
		vk::UniqueDescriptorSet m_DescriptorSet;
		std::unordered_map<bindableImage*, int> m_textureIndexMap;
		std::unordered_map<int, bindableImage*> m_indexTextureMap; // index 0 reserved for font atlas

		// vulkan has to have a dummy image to bind to the descriptor set by default to avoid null images bound
		bindableImage m_dummyImage;
		// pipeline
		vk::UniquePipeline m_pipeline;
		vk::UniquePipeline m_imagePipeline;

		font m_defaultFont;
		UIVertexBuffer m_imageVertexBuffer;
		UIVertexBuffer m_UIVertexBuffer;
		UIVertexBuffer m_debugDrawUIVertexBuffer;
		// simple shapes ui
		bindableImage m_emptyImage;
		char* m_BufferMappedMemory;
		float widthFactor;
		float heightFactor;
	};
}