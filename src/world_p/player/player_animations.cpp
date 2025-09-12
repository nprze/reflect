#include "player_animations.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"
#include "renderer_p/buffer/vulkan_buffer.h"
#include "assets/assets_manager.h"
#include <vma/vk_mem_alloc.h>
#include "player.h"



#define RFCT_PLAYER_ANIMATIONS_VERTEX_BUFFER_COUNT 1
#define RFCT_PLAYER_ANIMATIONS_VERTEX_BUFFER_TRIANGLE_COUNT 10000


rfct::playerAnimations rfct::playerAnimations::instance;

rfct::vulkanBufferLocation rfct::playerAnimations::requestVulkanBuffer(uint32_t triangleCount)
{
	for (uint32_t i = 0; i < RFCT_PLAYER_ANIMATIONS_VERTEX_BUFFER_COUNT; ++i) {
		if (trianglesLeftInBuffer[i] >= triangleCount) {
			uint32_t val = static_cast<uint32_t>(((RFCT_PLAYER_ANIMATIONS_VERTEX_BUFFER_TRIANGLE_COUNT * 3 * sizeof(Vertex)) - (trianglesLeftInBuffer[i]) * 3 * sizeof(Vertex)));
			trianglesLeftInBuffer[i] -= triangleCount;
 			return {&vulkanBuffers[i], val };
		}
	}
	RFCT_CRITICAL("cannot find vertex buffer to accomodate needs for animtaion");
}
void rfct::playerAnimations::loadAnimations()
{
	vulkanBuffers = (VulkanBuffer*)malloc(RFCT_PLAYER_ANIMATIONS_VERTEX_BUFFER_COUNT * sizeof(VulkanBuffer));
	trianglesLeftInBuffer.reserve(RFCT_PLAYER_ANIMATIONS_VERTEX_BUFFER_COUNT);
	for (uint32_t i = 0; i < RFCT_PLAYER_ANIMATIONS_VERTEX_BUFFER_COUNT; ++i) {
		new (&vulkanBuffers[i]) VulkanBuffer(RFCT_PLAYER_ANIMATIONS_VERTEX_BUFFER_TRIANGLE_COUNT * 3 * sizeof(Vertex), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, VMA_MEMORY_USAGE_GPU_ONLY);
		trianglesLeftInBuffer.push_back(RFCT_PLAYER_ANIMATIONS_VERTEX_BUFFER_TRIANGLE_COUNT);
	}
	m_idle = AssetsManager::get().loadAnimation("player/walkAnim/idle.txt");
	m_walking = AssetsManager::get().loadAnimation("player/walkAnim/walk.txt");
	m_jumpStart = AssetsManager::get().loadAnimation("player/walkAnim/jump-start.txt");
	m_jumpUp = AssetsManager::get().loadAnimation("player/walkAnim/jump-up.txt");
	m_jumpTurnover = AssetsManager::get().loadAnimation("player/walkAnim/jump-turnover.txt");
	m_jumpFall = AssetsManager::get().loadAnimation("player/walkAnim/jump-fall.txt");
	m_jumpReturn = AssetsManager::get().loadAnimation("player/walkAnim/jump-return.txt");
	m_dash = AssetsManager::get().loadAnimation("player/walkAnim/dash.txt");
	m_dash.cycleTime = dashFullTime;
	m_dashUp = AssetsManager::get().loadAnimation("player/walkAnim/dash-up.txt");
	m_dashUp.cycleTime = dashFullTime;
	m_hold = AssetsManager::get().loadAnimation("player/walkAnim/hold.txt");
	m_climb = AssetsManager::get().loadAnimation("player/walkAnim/climb.txt");
	
	m_currentAnimation = &m_idle;
}

void rfct::playerAnimations::unloadAnimations()
{
	for (uint32_t i = 0; i < RFCT_PLAYER_ANIMATIONS_VERTEX_BUFFER_COUNT; ++i) {
		vulkanBuffers[i].cleanup();
	}
	free(vulkanBuffers);
	vulkanBuffers = nullptr;
}

void rfct::playerAnimations::update(const glm::vec2& playerVel, const glm::vec2& playerPos, frameContext& ctx, entity player)
{
	RFCT_PROFILE_SCOPE("player animation update");

	const playerStateComponent* ps = player.get<playerStateComponent>();
	const inputVelocityComponent* ivel = player.get<inputVelocityComponent>();
	const velocityComponent* pvel = player.get<velocityComponent>();
	dashRotationAnimationTime = std::clamp(dashRotationAnimationTime - ctx.dt, 0.f, 10.f);
	if (dashRotationAnimationTime > 0.f) {
		float x = std::clamp((dashFullTime - dashRotationAnimationTime) / dashFullTime, 0.f, 1.f); // dash progress
		float rot = (angleMax) * (-std::pow(x, 1) + 1);
		player.get_mut<rotationComponent>()->rotation.z = rot;

	}
	else {
		player.get_mut<rotationComponent>()->rotation.z = 0.0f;
	}
	constexpr float velocityTreshold = 0.03f;
	switch (ps->state) {
	case playerState::normal: {
		if (pvel->velocity.y < -velocityTreshold) {
			// falling
			changeIfNotCurrent(&m_jumpFall);
		}
		else {
			ifThisChangeToThat(&m_jumpFall, &m_jumpReturn);
			if (!isThisPlaying(&m_jumpReturn)) {
				if (std::abs(ivel->velocity.x) > 0.1f) {
					changeIfNotCurrent(&m_walking);
				}
				else {
					changeIfNotCurrent(&m_idle);
				}
			}
		}
		break;
	}
	case playerState::jumping: {
		if (pvel->velocity.y != 0.f || ivel->velocity.y != 0.f) {
			if (!isAnyJumpAnimPlaying()) changeAnimation(&m_jumpUp);
			if (between(pvel->velocity.y, 0.6f, 3.f)) { changeIfNotCurrent(&m_jumpUp); }
			if (between(pvel->velocity.y, -0.3f, 0.6f)) { ifThisChangeToThat(&m_jumpUp, &m_jumpTurnover); }
			if (between(pvel->velocity.y, -10.f, -0.3f)) { ifThisChangeToThat(&m_jumpTurnover , &m_jumpFall); }
		}
		else {
			if (isAnyJumpAnimPlaying()) { 
				changeIfNotCurrent(&m_jumpReturn);
			}
		}
		break;
	}
	case playerState::dashing: {
		glm::vec2 dir = playerController::get().dashVelocity;
		if (dir.x == 0.f && dir.y!= 0.f) {
			changeIfNotCurrent(&m_dashUp);
		}
		else {
			changeIfNotCurrent(&m_dash);
		}
		break;
		if (dir.x == 0 && dir.y == 0) break;
		if (dir.x < 0) dir.x *= -1.f;
		angleMax = std::atan2(dir.x, dir.y) + (0.5f * 3.14f);
		dashRotationAnimationTime = dashFullTime;
		break;
	}
	case playerState::holdingVines: {
		changeIfNotCurrent(&m_hold);
		break;
	}
	case playerState::holdingBlocks: {
		if (pvel->velocity.y != 0.f) {
			changeIfNotCurrent(&m_climb);
		}
		else {
			changeIfNotCurrent(&m_hold);
		}
		break;
	}
	default: {
		RFCT_CRITICAL("state unknown: {}", (uint8_t)ps->state);
	}
	}
	/*
	const playerDashStateComponent* dc = player.get<playerDashStateComponent>();
	if (dc->dashing) {
		if (m_currentAnimation != &m_dashStart && m_currentAnimation != &m_dashEnd) {
			changeAnimation(&m_dashStart);
		}
		else {
			if (m_currentAnimation->endedPlaying) {
				changeAnimation(&m_dashEnd);
			}
		}
	}
	else {
		if (!player.get<playerStateComponent>()->grounded) {
			if (m_currentAnimation == &m_walking || m_currentAnimation == &m_idle) {
				// change anim
				changeAnimation(&m_jumpStart);
			}
			else {
				if (m_currentAnimation == &m_jumpStart) {
					if (m_jumpStart.endedPlaying) {
						changeAnimation(&m_jumpUp);
					}
				}
				else {
					if (m_currentAnimation == &m_jumpUp) {
						if (playerVel.y < 1.f) {
							changeAnimation(&m_jumpTurnover);
						}
					}
					else {
						if (m_currentAnimation == &m_jumpTurnover) {
							if (playerVel.y <= 0.1f) {
								changeAnimation(&m_jumpFall);
							}

						}
					}
				}
			}
		}
		else {
			if (m_currentAnimation == &m_jumpFall) {
				changeAnimation(&m_jumpReturn);
			}
			else {
				if (m_currentAnimation == &m_jumpReturn && !m_jumpReturn.endedPlaying) {
					// just update normally
				}
				else {
					if (std::abs(playerVel.x) > 0.1f) {
						if (m_currentAnimation != &m_walking) {
							changeAnimation(&m_walking);
						}
					}
					else {
						if (m_currentAnimation != &m_idle) {
							changeAnimation(&m_idle);
						}
					}
				}
			}
		}
	}*/

	m_timeSinceFrameChanged += ctx.dt;
	if (m_timeSinceFrameChanged > m_currentAnimation->timePerFrame) {
		if (!m_currentAnimation->shouldBeRepeated&& m_currentFrame == m_currentAnimation->frameCount-1) {
			m_currentAnimation->endedPlaying = true;
		}
		else {
			// loop
			m_timeSinceFrameChanged = std::fmod(m_timeSinceFrameChanged, m_currentAnimation->timePerFrame);

			m_bufferOffset += m_currentAnimation->trianglesPerFrame[m_currentFrame] * 3 * sizeof(Vertex);
			m_currentFrame += 1;
			if (m_currentFrame > m_currentAnimation->frameCount - 1) {
				m_currentFrame = 0;
				m_bufferOffset = 0;
			}
		}
	}


	m_rightHairAnim.update(playerVel, ctx.fixedUpdateTimes);
	m_leftHairAnim.update(playerVel, ctx.fixedUpdateTimes);
	//m_rightHairAnim.draw(playerPos);
	//m_leftHairAnim.draw(playerPos);
}

void rfct::playerAnimations::changeAnimation(frameAnimation* newAnim)
{
	m_currentAnimation = newAnim;
	m_timeSinceFrameChanged = 0.f;
	m_bufferOffset = 0;
	m_currentFrame = 0;
	newAnim->endedPlaying = false;
}

void rfct::playerAnimations::drawPlayer(vk::CommandBuffer& cmdBffr)
{

	vk::Buffer vertexBuffers[] = { m_currentAnimation->buffer->buffer  };
	vk::DeviceSize offsets[] = { m_currentAnimation->bufferOffsetInBytes + m_bufferOffset };
	cmdBffr.bindVertexBuffers(0,1, vertexBuffers, offsets);
	cmdBffr.draw(m_currentAnimation->trianglesPerFrame[m_currentFrame] * 3, 1, 0, 0);
}

void rfct::playerAnimations::initHairAnim(float playerWidth, float playerHeight)
{

	m_rightHairAnim.init(glm::vec2{ playerWidth * 0.4f, 0.4f * playerHeight }, 0.9f * playerHeight, 4);
	m_leftHairAnim.init(glm::vec2{ playerWidth * -0.4f, 0.4f * playerHeight }, 0.9f * playerHeight, 4);
}

void rfct::playerAnimations::changeIfNotCurrent(frameAnimation* newAnim)
{
	if (m_currentAnimation != newAnim) {
		changeAnimation(newAnim);
	}
}

bool rfct::playerAnimations::ifThisChangeToThat(frameAnimation* checkAnim, frameAnimation* newAnim)
{
	if (m_currentAnimation == checkAnim) {
		changeAnimation(newAnim);
		return true;
	}
	return false;
}

bool rfct::playerAnimations::isThisPlaying(frameAnimation* checkAnim)
{
	return (m_currentAnimation == checkAnim && !m_currentAnimation->endedPlaying);
}

bool rfct::playerAnimations::isAnyJumpAnimPlaying()
{
	return
		isThisPlaying(&m_jumpStart) ||
		isThisPlaying(&m_jumpUp) ||
		isThisPlaying(&m_jumpTurnover) ||
		isThisPlaying(&m_jumpFall) ||
		isThisPlaying(&m_jumpReturn);
}
