#include "render_target_manager.h"
#include "assets/assets_utils.h"
#include "renderer_p/components/renderer_components.h"

void rfct::RfctRenderImagesManager::CreateImages(vk::SampleCountFlagBits msaaSamples, rfct::RfctDevice& deviceWrapper, RfctVulkanInstance& instanceWrapper, rfct::RfctQueue& queueWrapper,
    RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper) {
    RFCT_PROFILE_FUNCTION();
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
		imageSpec.imageSamples = msaaSamples;
        imageSpec.debugName = "msaa color image " + std::to_string(i);
		m_msaaColorImages[i].CreateImageAndView(imageSpec, deviceWrapper, instanceWrapper, queueWrapper, allocatorWrapper);
    }
}

void rfct::RfctRenderImagesManager::CreateFrameBuffers(RfctSwapChain& swapChainWrapper, vk::Device device) {
    RFCT_PROFILE_FUNCTION();
    std::vector<RfctRenderImage*> attachments;
	attachments.reserve(2);

    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
        attachments = { &m_msaaColorImages[i], &m_sceneImages[i] };
		m_sceneImages[i].InitFrameBuffer(attachments, m_sceneRenderPass.get(), device);
        attachments = { &m_swapchainImages[i] };
		m_swapchainImages[i].InitFrameBuffer(attachments, m_UIRenderPass.get(), device);
        attachments = { &m_bloom1Images[i] };
		m_bloom1Images[i].InitFrameBuffer(attachments, m_UIRenderPass.get(), device);
        attachments = { &m_bloom2Images[i] };
		m_bloom2Images[i].InitFrameBuffer(attachments, m_UIRenderPass.get(), device);
    }
}

void rfct::RfctRenderImagesManager::CleanupResources(RfctVulkanMemAllocator& allocatorWrapper, vk::Device& device) {
    RFCT_PROFILE_FUNCTION();
    for (uint32_t i = 0; i < m_bloom1Images.size(); i++) {
		m_bloom1Images[i].Cleanup(allocatorWrapper, device);
		m_bloom2Images[i].Cleanup(allocatorWrapper, device);
		m_sceneImages[i].Cleanup(allocatorWrapper, device);
		m_msaaColorImages[i].Cleanup(allocatorWrapper, device);
    }
}

rfct::RfctRenderImagesManager::RfctRenderImagesManager(rfct::RfctDevice& deviceWrapper, RfctVulkanInstance& instanceWrapper, rfct::RfctQueue& queueWrapper,
    RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper) {
    CreateRenderPasses(deviceWrapper.GetDevice());
    CreateResources(deviceWrapper, instanceWrapper, queueWrapper, allocatorWrapper, swapChainWrapper);
}

void rfct::RfctRenderImagesManager::CreateResources(rfct::RfctDevice& deviceWrapper, RfctVulkanInstance& instanceWrapper, rfct::RfctQueue& queueWrapper,
    RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper) {
    // Swapchain images
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

    // Other images
    CleanupResources(allocatorWrapper, deviceWrapper.GetDevice());        
    CreateImages(vk::SampleCountFlagBits::e4, deviceWrapper, instanceWrapper, queueWrapper, allocatorWrapper, swapChainWrapper);
    CreateFrameBuffers(swapChainWrapper, deviceWrapper.GetDevice());
}
