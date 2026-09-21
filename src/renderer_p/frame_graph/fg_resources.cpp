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

void rfct::RfctFrameGraphPerFrameResources::CreateUniformBuffers(RfctVulkanMemAllocator& memAllocatorWrapper, vk::Device device) {
	m_sceneUniform.CreateUniformBuffer(memAllocatorWrapper, device);
	m_UIUniform.CreateUniformBuffer(memAllocatorWrapper, device);
}

void rfct::RfctFrameGraphPerFrameResources::DestroyUniformBuffers() {
	m_sceneUniform.DestroyUniformBuffer();
	m_UIUniform.DestroyUniformBuffer();
}

void rfct::RfctFrameGraphResources::PreFrame(const frameContext& ctx, float changeSceneEffectMultiplier) {
	RfctUniformData cameraData;
	cameraData.vp = getVPMatrix();
	cameraData.globalTime = ctx.globalTime;
	cameraData.changeSceneEffectMultiplier = changeSceneEffectMultiplier;
	m_perFrameResources[ctx.frameInFlightIndex].GetSceneUniformBuffer().UpdateUniformData(cameraData);
	cameraData.vp = getUIMatrix({ 400, 400 }); // TODO: fix extent getting 
	m_perFrameResources[ctx.frameInFlightIndex].GetUIUniformBuffer().UpdateUniformData(cameraData);
}

void rfct::RfctFrameGraphResources::CreateUniformBuffers(RfctVulkanMemAllocator& memAllocatorWrapper, vk::Device device) {
	for (uint32_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; i++) {
		m_perFrameResources[i].CreateUniformBuffers(memAllocatorWrapper, device);
	}
}

void rfct::RfctFrameGraphResources::CreateRenderPasses(vk::Device device) {
    RFCT_PROFILE_FUNCTION();
    {
        RfctRenderPass::RfctRenderPassSpec spec;
        spec.colorAttachmentDesc.format = vk::Format::eB8G8R8A8Unorm;
        spec.colorAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
        spec.colorAttachmentDesc.loadOp = vk::AttachmentLoadOp::eLoad;
        spec.colorAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
        spec.colorAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        spec.colorAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        spec.colorAttachmentDesc.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
        spec.colorAttachmentDesc.finalLayout = vk::ImageLayout::ePresentSrcKHR;
        m_UIRenderPass.CreateRenderPass(spec, device);
    }
    {
        RfctRenderPass::RfctRenderPassSpec spec;
        spec.colorAttachmentDesc.format = vk::Format::eB8G8R8A8Unorm;
        spec.colorAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
        spec.colorAttachmentDesc.loadOp = vk::AttachmentLoadOp::eLoad;
        spec.colorAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
        spec.colorAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        spec.colorAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        spec.colorAttachmentDesc.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
        spec.colorAttachmentDesc.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
        m_IntermediateRenderPass.CreateRenderPass(spec, device);
    }
    {
        RfctRenderPass::RfctRenderPassSpec spec;
        spec.colorAttachmentDesc.format = vk::Format::eB8G8R8A8Unorm;
        spec.colorAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
        spec.colorAttachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
        spec.colorAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
        spec.colorAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        spec.colorAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        spec.colorAttachmentDesc.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
        spec.colorAttachmentDesc.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
        m_IntermediateClearRenderPass.CreateRenderPass(spec, device);
    }
    {
        RfctRenderPass::RfctRenderPassSpec spec;
        spec.colorAttachmentDesc.format = vk::Format::eB8G8R8A8Unorm;
        spec.colorAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
        spec.colorAttachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
        spec.colorAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
        spec.colorAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        spec.colorAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        spec.colorAttachmentDesc.initialLayout = vk::ImageLayout::ePresentSrcKHR;
        spec.colorAttachmentDesc.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
        m_presentToColorAttachment.CreateRenderPass(spec, device);
    }
    {
        RfctRenderPass::RfctRenderPassSpec spec;
        spec.colorAttachmentDesc.format = vk::Format::eB8G8R8A8Unorm;
        spec.colorAttachmentDesc.samples = vk::SampleCountFlagBits::e4;
        spec.colorAttachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
        spec.colorAttachmentDesc.storeOp = vk::AttachmentStoreOp::eDontCare;
        spec.colorAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        spec.colorAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        spec.colorAttachmentDesc.initialLayout = vk::ImageLayout::eUndefined;
        spec.colorAttachmentDesc.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

        spec.resolveAttachmentDesc.format = vk::Format::eB8G8R8A8Unorm;
        spec.resolveAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
        spec.resolveAttachmentDesc.loadOp = vk::AttachmentLoadOp::eDontCare;
        spec.resolveAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
        spec.resolveAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        spec.resolveAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        spec.resolveAttachmentDesc.initialLayout = vk::ImageLayout::eUndefined;
        spec.resolveAttachmentDesc.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
        m_sceneRenderPass.CreateRenderPass(spec, device);
    }
}

void rfct::RfctFrameGraphResources::DestroyResources(vk::Device device) {
	for (uint32_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; i++) {
		m_perFrameResources[i].DestroyUniformBuffers();
	}
    m_UIRenderPass.DestroyRenderPass(device);
    m_presentToColorAttachment.DestroyRenderPass(device);
    m_IntermediateClearRenderPass.DestroyRenderPass(device);
    m_IntermediateRenderPass.DestroyRenderPass(device);
    m_sceneRenderPass.DestroyRenderPass(device);
}
