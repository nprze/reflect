#pragma once
#include "assets/frame_animation.h"
namespace rfct {
	struct vulkanBufferLocation {
		VulkanBuffer* buffer;
		uint32_t offsetInBytes;
	};
	class VulkanBuffer;
	class playerAnimations {
	private:
		static playerAnimations instance;
	public:
		static playerAnimations& get() { return instance; }
	private:
		VulkanBuffer* vulkanBuffers;
		std::vector<uint32_t> trianglesLeftInBuffer;
	public:
		vulkanBufferLocation requestVulkanBuffer(uint32_t triangleCount);
		void loadAnimations();
		void unloadAnimations();
		void update(float dt);
		void changeAnimation(animation* newAnim);
		void drawPlayer(vk::CommandBuffer& cmdBffr);
	private:
		uint32_t m_currentFrame;
		float m_timeSinceFrameChanged;
		uint32_t m_bufferOffset;
		animation m_walking;
		animation* m_currentAnimation;
	};
}