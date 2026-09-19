#include "fg_resources.h"
#include "world_p/camera/camera.h"
#include "context.h"
#include "world_p/world.h"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"

glm::mat4 getUIMatrix(vk::Extent2D extent) {
	RFCT_PROFILE_FUNCTION();
	glm::mat4 screenRot = glm::rotate(glm::mat4(1), glm::radians(rfct::world::getWorld().screenViewTransformDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	return screenRot * glm::ortho(0.0f, static_cast<float>(extent.width), 0.0f, static_cast<float>(extent.height));
}

void rfct::RfctFrameGraphResources::CreateUniformBuffers(RfctVulkanMemAllocator& memAllocatorWrapper, vk::Device device) {
	m_sceneUniform.CreateUniformBuffer(memAllocatorWrapper, device);
	m_UIUniform.CreateUniformBuffer(memAllocatorWrapper, device);
}

void rfct::RfctFrameGraphResources::PreFrame(const frameContext& ctx, float changeSceneEffectMultiplier) {
	RfctUniformData cameraData;
	cameraData.vp = getVPMatrix();
	cameraData.globalTime = ctx.globalTime;
	cameraData.changeSceneEffectMultiplier = changeSceneEffectMultiplier;
	m_sceneUniform.UpdateUniformData(cameraData);
	cameraData.vp = getUIMatrix({ 400, 400 }); // TODO: fix extent getting 
	m_UIUniform.UpdateUniformData(cameraData);
}