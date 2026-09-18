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

void rfct::RfctRenderImagesManager::CreateRenderPasses(vk::Device device, vk::SampleCountFlagBits msaaSamples) {
    RFCT_PROFILE_FUNCTION();
    {
        vk::AttachmentDescription colorAttachment = {};
        colorAttachment.format = vk::Format::eB8G8R8A8Unorm;
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
        colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass = {};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        vk::SubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.srcAccessMask = vk::AccessFlagBits::eNone;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        vk::SubpassDependency dependency2 = {};
        dependency2.srcSubpass = 0;
        dependency2.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependency2.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency2.dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
        dependency2.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        dependency2.dstAccessMask = vk::AccessFlagBits::eNone;
        dependency2.dependencyFlags = vk::DependencyFlagBits::eByRegion;

        vk::RenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        std::array<vk::SubpassDependency, 2> dependencies = { dependency, dependency2 };
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        m_UIRenderPass = device.createRenderPassUnique(renderPassInfo).value;
    }
    {
        vk::AttachmentDescription colorAttachment = {};
        colorAttachment.format = vk::Format::eB8G8R8A8Unorm;
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
        colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass = {};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        vk::SubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.srcAccessMask = vk::AccessFlagBits::eNone;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        vk::SubpassDependency dependency2 = {};
        dependency2.srcSubpass = 0;
        dependency2.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependency2.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency2.dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
        dependency2.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        dependency2.dstAccessMask = vk::AccessFlagBits::eNone;
        dependency2.dependencyFlags = vk::DependencyFlagBits::eByRegion;

        vk::RenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        std::array<vk::SubpassDependency, 2> dependencies = { dependency, dependency2 };
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        m_IntermediateRenderPass = device.createRenderPassUnique(renderPassInfo).value;
    } 
    {
        vk::AttachmentDescription colorAttachment = {};
        colorAttachment.format = vk::Format::eB8G8R8A8Unorm;
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
        colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass = {};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        vk::SubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.srcAccessMask = vk::AccessFlagBits::eNone;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        vk::SubpassDependency dependency2 = {};
        dependency2.srcSubpass = 0;
        dependency2.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependency2.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency2.dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
        dependency2.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        dependency2.dstAccessMask = vk::AccessFlagBits::eNone;
        dependency2.dependencyFlags = vk::DependencyFlagBits::eByRegion;

        vk::RenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        std::array<vk::SubpassDependency, 2> dependencies = { dependency, dependency2 };
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        m_IntermediateClearRenderPass = device.createRenderPassUnique(renderPassInfo).value;
    } 
    {
        vk::AttachmentDescription colorAttachment = {};
        colorAttachment.format = vk::Format::eB8G8R8A8Unorm;
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::ePresentSrcKHR;
        colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass = {};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        vk::SubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.srcAccessMask = vk::AccessFlagBits::eNone;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        vk::SubpassDependency dependency2 = {};
        dependency2.srcSubpass = 0;
        dependency2.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependency2.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency2.dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
        dependency2.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        dependency2.dstAccessMask = vk::AccessFlagBits::eNone;
        dependency2.dependencyFlags = vk::DependencyFlagBits::eByRegion;

        vk::RenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        std::array<vk::SubpassDependency, 2> dependencies = { dependency, dependency2 };
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        m_presentToColorAttachment = device.createRenderPassUnique(renderPassInfo).value;
    }
    {
        vk::AttachmentDescription colorAttachment = {};
        colorAttachment.format = vk::Format::eB8G8R8A8Unorm;
        colorAttachment.samples = msaaSamples;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentDescription resolveAttachment = {};
        resolveAttachment.format = vk::Format::eB8G8R8A8Unorm;
        resolveAttachment.samples = vk::SampleCountFlagBits::e1;
        resolveAttachment.loadOp = vk::AttachmentLoadOp::eDontCare;
        resolveAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        resolveAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        resolveAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        resolveAttachment.initialLayout = vk::ImageLayout::eUndefined;
        resolveAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentReference resolveAttachmentRef = {};
        resolveAttachmentRef.attachment = 1;
        resolveAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass = {};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pResolveAttachments = &resolveAttachmentRef;

        vk::SubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.srcAccessMask = vk::AccessFlagBits::eNone;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        vk::SubpassDependency dependency2 = {};
        dependency2.srcSubpass = 0;
        dependency2.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependency2.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency2.dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
        dependency2.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        dependency2.dstAccessMask = vk::AccessFlagBits::eNone;
        dependency2.dependencyFlags = vk::DependencyFlagBits::eByRegion;

        std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, resolveAttachment };

        vk::RenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        std::array<vk::SubpassDependency, 2> dependencies = { dependency, dependency2 };
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        m_sceneRenderPass = device.createRenderPassUnique(renderPassInfo).value;
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
