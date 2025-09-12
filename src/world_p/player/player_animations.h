#pragma once
#include "assets/frame_animation.h"
#include "hair_anim.h"
#include "context.h"

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
		void update(const glm::vec2& playerVel, const glm::vec2& playerPos, frameContext& ctx, entity player);
		void changeAnimation(frameAnimation* newAnim);
		void drawPlayer(vk::CommandBuffer& cmdBffr);
		void initHairAnim(float playerWidth, float playerHeight);
	private:
		uint32_t m_currentFrame;
		float m_timeSinceFrameChanged;
		uint32_t m_bufferOffset;

		frameAnimation m_idle;
		frameAnimation m_walking;
		frameAnimation m_jumpStart;
		frameAnimation m_jumpUp;
		frameAnimation m_jumpTurnover;
		frameAnimation m_jumpFall;
		frameAnimation m_jumpReturn;
		frameAnimation m_dash;
		frameAnimation m_dashUp;
		frameAnimation m_hold;
		frameAnimation m_climb;

		frameAnimation* m_currentAnimation;

		hairAnimation m_rightHairAnim;
		hairAnimation m_leftHairAnim;

		float dashRotationAnimationTime = 0;
		float angleMax = 0;

		void changeIfNotCurrent(frameAnimation* newAnim);
		bool ifThisChangeToThat(frameAnimation* checkAnim, frameAnimation* newAnim);
		bool isThisPlaying(frameAnimation* checkAnim);

		inline bool between(float value, float min, float max) { return value >= min && value <= max; };
		bool isAnyJumpAnimPlaying();
		
	};
}