#include "fg_resources.h"
#include "world_p/camera/camera.h"
#include "context.h"
#include "world_p/world.h"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "renderer_p/components/renderer_components.h"

glm::mat4 getUIMatrix(vk::Extent2D extent) {
	RFCT_PROFILE_FUNCTION();
	glm::mat4 screenRot = glm::rotate(glm::mat4(1), glm::radians(rfct::world::getWorld().screenViewTransformDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	return screenRot * glm::ortho(0.0f, static_cast<float>(extent.width), 0.0f, static_cast<float>(extent.height));
}

void rfct::RfctFrameGraphPerFrameResources::CreateUniformBuffers(RfctVulkanMemAllocator& memAllocatorWrapper, vk::Device device) {
	m_sceneUniform.CreateUniformBuffer(memAllocatorWrapper, device);
	m_UIUniform.CreateUniformBuffer(memAllocatorWrapper, device);
}

void rfct::RfctFrameGraphPerFrameResources::CreateSynchronizationStructures(RfctVulkanMemAllocator& allocatorWrapper, RfctQueue& queue, vk::Device device) {
    RFCT_PROFILE_FUNCTION();
    vk::CommandPoolCreateInfo poolInfo{
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        queue.GetGraphicsQueueFamilyIndex()
    };
    m_commandPool = device.createCommandPool(poolInfo).value;

    vk::CommandBufferAllocateInfo allocInfoScene{ m_commandPool, vk::CommandBufferLevel::ePrimary, 1 };
    auto commandBuffersScene = device.allocateCommandBuffers(allocInfoScene);
    m_commandBuffer = std::move(commandBuffersScene.value[0]);

    vk::FenceCreateInfo fenceInfo{ vk::FenceCreateFlagBits::eSignaled };
    m_thisFrameRenderFinishedFence = device.createFence(fenceInfo).value;

    vk::SemaphoreCreateInfo semaphoreInfo{};
    m_ImageAvaibleSemaphore = device.createSemaphore(semaphoreInfo).value;
}

void rfct::RfctFrameGraphPerFrameResources::DestroyUniformBuffers() {
	m_sceneUniform.DestroyUniformBuffer();
	m_UIUniform.DestroyUniformBuffer();
}

void rfct::RfctFrameGraphPerFrameResources::WaitImageRenderFinished(vk::Device device) {
    RFCT_PROFILE_FUNCTION();
    RFCT_VULKAN_CHECK(device.waitForFences(1, &m_thisFrameRenderFinishedFence, VK_TRUE, UINT64_MAX));
}

void rfct::RfctFrameGraphPerFrameResources::ResetFences(vk::Device device) {
    RFCT_PROFILE_FUNCTION();
    RFCT_VULKAN_CHECK(device.resetFences(1, &m_thisFrameRenderFinishedFence));
}

vk::SubmitInfo rfct::RfctFrameGraphPerFrameResources::GetSubmitInfo() const {
    return vk::SubmitInfo()
        .setWaitSemaphores(m_ImageAvaibleSemaphore)
        .setCommandBuffers(m_commandBuffer);
        // .setSignalSemaphores(m_renderFinishedSemaphore);
}



void rfct::RfctFrameGraphResources::PreFrame(const RfctFrameContext& ctx, float changeSceneEffectMultiplier) {
	RfctUniformData cameraData;
	cameraData.vp = getVPMatrix();
	cameraData.globalTime = ctx.globalTime;
	cameraData.changeSceneEffectMultiplier = changeSceneEffectMultiplier;
	m_perFrameResources[ctx.frameInFlightIndex].GetSceneUniformBuffer().UpdateUniformData(cameraData);
	cameraData.vp = getUIMatrix({ 400, 400 }); // TODO: fix extent getting 
	m_perFrameResources[ctx.frameInFlightIndex].GetUIUniformBuffer().UpdateUniformData(cameraData);
}

void rfct::RfctFrameGraphResources::CreateResources(RfctDevice& deviceWrapper, RfctVulkanInstance& instanceWrapper, RfctQueue& queueWrapper, 
    RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper) {
    RFCT_PROFILE_FUNCTION();
    CreateUniformBuffers(allocatorWrapper, deviceWrapper.GetDevice());

    CreateRenderPasses(deviceWrapper.GetDevice());

    DestroyImages(allocatorWrapper, deviceWrapper.GetDevice());
    CreateImages(deviceWrapper, instanceWrapper, queueWrapper, allocatorWrapper, swapChainWrapper);
    CreateFrameBuffers(swapChainWrapper, deviceWrapper.GetDevice());
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

void rfct::RfctFrameGraphResources::CreateImages(rfct::RfctDevice& deviceWrapper, RfctVulkanInstance& instanceWrapper, rfct::RfctQueue& queueWrapper,
    RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper) {
    RFCT_PROFILE_FUNCTION();

    m_swapchainImages.resize(RFCT_FRAMES_IN_FLIGHT + 1);
    auto swapChainImagesResult = deviceWrapper.GetDevice().getSwapchainImagesKHR(swapChainWrapper.GetSwapChain());
    RFCT_VULKAN_CHECK(swapChainImagesResult.result);
    std::vector<vk::Image> swapChainImages = swapChainImagesResult.value;
    RfctRenderImage::RfctRenderImageSpec spec;
    spec.allocateImage = false;
    spec.dafaultFormat = swapChainWrapper.GetSurfaceFormat().format;
    spec.dafaultLayout = vk::ImageLayout::ePresentSrcKHR;
    spec.extent = swapChainWrapper.GetExtent();
    for (uint32_t i = 0; i < RFCT_FRAMES_IN_FLIGHT + 1; i++) {
        spec.debugName = "Swapchain Image" + std::to_string(i);
        spec.image = swapChainImages[i];
        m_swapchainImages[i].CreateImageAndView(spec, deviceWrapper, instanceWrapper, queueWrapper, allocatorWrapper);
    }

    m_sceneImages.resize(m_swapchainImages.size());
    m_bloom1Images.resize(m_swapchainImages.size());
    m_bloom2Images.resize(m_swapchainImages.size());
    m_msaaColorImages.resize(m_swapchainImages.size());

    RfctRenderImage::RfctRenderImageSpec imageSpec;
    imageSpec.extent = swapChainWrapper.GetExtent();
    imageSpec.dafaultFormat = swapChainWrapper.GetSurfaceFormat().format;
    imageSpec.dafaultLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imageSpec.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
        imageSpec.imageSamples = vk::SampleCountFlagBits::e1;
        imageSpec.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
        imageSpec.debugName = "bloom1 image " + std::to_string(i);
        m_bloom1Images[i].CreateImageAndView(imageSpec, deviceWrapper, instanceWrapper, queueWrapper, allocatorWrapper);
        imageSpec.debugName = "bloom2 image " + std::to_string(i);
        m_bloom2Images[i].CreateImageAndView(imageSpec, deviceWrapper, instanceWrapper, queueWrapper, allocatorWrapper);
        imageSpec.debugName = "sceneImage " + std::to_string(i);
        m_sceneImages[i].CreateImageAndView(imageSpec, deviceWrapper, instanceWrapper, queueWrapper, allocatorWrapper);

        // msaa resources
        imageSpec.usage = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment;
        imageSpec.imageSamples = vk::SampleCountFlagBits::e4;
        imageSpec.debugName = "msaa color image " + std::to_string(i);
        m_msaaColorImages[i].CreateImageAndView(imageSpec, deviceWrapper, instanceWrapper, queueWrapper, allocatorWrapper);
    }
}

void rfct::RfctFrameGraphResources::CreateFrameBuffers(RfctSwapChain& swapChainWrapper, vk::Device device) {
    RFCT_PROFILE_FUNCTION();

    std::vector<RfctRenderImage*> attachments;
    attachments.reserve(2);

    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
        attachments = { &m_msaaColorImages[i], &m_sceneImages[i] };
        m_sceneImages[i].InitFrameBuffer(attachments, m_sceneRenderPass.GetPass(), device);
        attachments = { &m_swapchainImages[i] };
        m_swapchainImages[i].InitFrameBuffer(attachments, m_UIRenderPass.GetPass(), device);
        attachments = { &m_bloom1Images[i] };
        m_bloom1Images[i].InitFrameBuffer(attachments, m_UIRenderPass.GetPass(), device);
        attachments = { &m_bloom2Images[i] };
        m_bloom2Images[i].InitFrameBuffer(attachments, m_UIRenderPass.GetPass(), device);
    }
}

void rfct::RfctFrameGraphResources::DestroyResources(RfctVulkanMemAllocator& allocatorWrapper, vk::Device& device) {
    RFCT_PROFILE_FUNCTION();
	for (uint32_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; i++) {
		m_perFrameResources[i].DestroyUniformBuffers();
	}
    DestroyImages(allocatorWrapper, device);
    m_UIRenderPass.DestroyRenderPass(device);
    m_presentToColorAttachment.DestroyRenderPass(device);
    m_IntermediateClearRenderPass.DestroyRenderPass(device);
    m_IntermediateRenderPass.DestroyRenderPass(device);
    m_sceneRenderPass.DestroyRenderPass(device);
}

void rfct::RfctFrameGraphResources::DestroyImages(RfctVulkanMemAllocator& allocatorWrapper, vk::Device& device) {
    RFCT_PROFILE_FUNCTION();
    for (uint32_t i = 0; i < m_bloom1Images.size(); i++) {
        m_bloom1Images[i].Cleanup(allocatorWrapper, device);
        m_bloom2Images[i].Cleanup(allocatorWrapper, device);
        m_sceneImages[i].Cleanup(allocatorWrapper, device);
        m_msaaColorImages[i].Cleanup(allocatorWrapper, device);
    }
}
