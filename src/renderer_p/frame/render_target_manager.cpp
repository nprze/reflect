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

void rfct::RfctRenderImage::TransformLayoutSync(vk::ImageLayout newLayout, RfctDevice& deviceWrapper, RfctQueue& queue) {
    RFCT_PROFILE_FUNCTION();
    vk::CommandBufferAllocateInfo allocInfo(
        rfct::GetAssetsCommandPool(deviceWrapper),
        vk::CommandBufferLevel::ePrimary,
        1
    );
    auto cmdBuffersAllocResult = deviceWrapper.GetDevice().allocateCommandBuffers(allocInfo);
    RFCT_VULKAN_CHECK(cmdBuffersAllocResult.result);
    vk::CommandBuffer commandBuffer = cmdBuffersAllocResult.value[0];

    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    RFCT_VULKAN_CHECK(commandBuffer.begin(beginInfo));

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = m_currentLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    barrier.srcAccessMask = vk::AccessFlags{}; // oldLayout is always undefined

    if (newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    }
    else if (newLayout == vk::ImageLayout::ePresentSrcKHR) {
        barrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe;
    }
    else {
        RFCT_CRITICAL("Unsupported layout transition in transformImage");
    }

    commandBuffer.pipelineBarrier(
        sourceStage, destinationStage,
        vk::DependencyFlags{},
        nullptr, nullptr, barrier
    );

    RFCT_VULKAN_CHECK(commandBuffer.end());

    vk::SubmitInfo submitInfo({}, {}, commandBuffer);
    vk::FenceCreateInfo fenceInfo;
    auto fenceCreateResult = deviceWrapper.GetDevice().createFence(fenceInfo);
    RFCT_VULKAN_CHECK(fenceCreateResult.result);
    vk::Fence fence = fenceCreateResult.value;
    queue.SubmitGraphics(submitInfo, fence);
    RFCT_VULKAN_CHECK(deviceWrapper.GetDevice().waitForFences(fence, VK_TRUE, UINT64_MAX));
    
	m_currentLayout = newLayout;
    deviceWrapper.GetDevice().freeCommandBuffers(rfct::GetAssetsCommandPool(deviceWrapper), commandBuffer);
    deviceWrapper.GetDevice().destroyFence(fence);
}

void rfct::RfctRenderImage::TransformLayoutAsync(vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer) {
    RFCT_PROFILE_FUNCTION();
    vk::ImageSubresourceRange subresourceRange = {
        vk::ImageAspectFlagBits::eColor,
        0, 1,
        0, 1
    };
    vk::AccessFlags srcAccessMask;
    vk::AccessFlags dstAccessMask;
    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;

    if (m_currentLayout == vk::ImageLayout::eColorAttachmentOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        dstAccessMask = vk::AccessFlagBits::eShaderRead;
        srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (m_currentLayout == vk::ImageLayout::eShaderReadOnlyOptimal && newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
        srcAccessMask = vk::AccessFlagBits::eShaderRead;
        dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        srcStage = vk::PipelineStageFlagBits::eFragmentShader;
        dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    }
    else {
        RFCT_CRITICAL("Unsupported layout transition");
    }
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = m_currentLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange = subresourceRange;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;

    commandBuffer.pipelineBarrier(
        srcStage,
        dstStage,
        vk::DependencyFlags{},
        nullptr, nullptr,
        barrier
    );
	m_currentLayout = newLayout;
}

void rfct::RfctRenderImage::CreateImageAndView(const RfctRenderImageSpec& spec, RfctDevice& deviceWrapper, RfctVulkanInstance& instanceWrapper, RfctQueue& queueWrapper, RfctVulkanMemAllocator& allocatorWrapper) {
    RFCT_PROFILE_FUNCTION();
    m_format = spec.dafaultFormat;
    m_extent = spec.extent;
	m_debugName = spec.debugName;
	m_sampleCount = spec.imageSamples;
    if (spec.allocateImage) {
        AllocateImage(spec, deviceWrapper, queueWrapper, allocatorWrapper);
    } else {
        m_image = spec.image;
	}
    TransformLayoutSync(spec.dafaultLayout, deviceWrapper, queueWrapper);
    instanceWrapper.SetObjectName(m_image, m_debugName, vk::ObjectType::eImage, deviceWrapper.GetDevice());
	CreateImageView(deviceWrapper.GetDevice());
}

void rfct::RfctRenderImage::InitFrameBuffer(std::vector<RfctRenderImage*> attachments, vk::RenderPass renderPass, vk::Device device) {
    RFCT_PROFILE_FUNCTION();
	std::vector<vk::ImageView> imageViews;
	imageViews.reserve(attachments.size());
	vk::Extent2D extent = m_extent;
    for (size_t i = 0; i < attachments.size(); i++) {
		RFCT_ASSERT(attachments[i] != nullptr);
		imageViews.push_back(attachments[i]->m_imageView.get());
        RFCT_ASSERT(attachments[i]->m_extent == extent && attachments[i]->m_extent != vk::Extent2D(1, 1));
    }
    vk::FramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.renderPass = renderPass;
    frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
    frameBufferCreateInfo.pAttachments = imageViews.data();
    frameBufferCreateInfo.width = extent.width;
    frameBufferCreateInfo.height = extent.height;
    frameBufferCreateInfo.layers = 1;
    m_frameBuffer = device.createFramebufferUnique(frameBufferCreateInfo).value;
	hasFrameBuffer = true;
}

void rfct::RfctRenderImage::Cleanup(RfctVulkanMemAllocator& allocatorWrapper, vk::Device device) {
    if (wasAllocatedUsingVMA) {
        vmaDestroyImage(allocatorWrapper.GetAllocator(), static_cast<VkImage>(m_image), m_imageAllocation);
		wasAllocatedUsingVMA = false;
    }
}

void rfct::RfctRenderImage::AllocateImage(const RfctRenderImage::RfctRenderImageSpec& spec, RfctDevice& deviceWrapper, RfctQueue& queueWrapper, RfctVulkanMemAllocator& allocatorWrapper) {
    RFCT_PROFILE_FUNCTION();
    // Create Vulkan image
    vk::ImageCreateInfo imageInfo({}, vk::ImageType::e2D, m_format,
        { static_cast<uint32_t>(m_extent.width), static_cast<uint32_t>(m_extent.height), 1 }, 1, 1,
        spec.imageSamples, vk::ImageTiling::eOptimal,
        spec.usage,
        vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo imageAllocInfo{};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(allocatorWrapper.GetAllocator(), reinterpret_cast<const VkImageCreateInfo*>(&imageInfo), &imageAllocInfo,
        reinterpret_cast<VkImage*>(&m_image), &m_imageAllocation, nullptr) != VK_SUCCESS) {
        RFCT_CRITICAL("Failed to create Vulkan image");
    }
    wasAllocatedUsingVMA = true;
}

void rfct::RfctRenderImage::CreateImageView(vk::Device device) {
    RFCT_PROFILE_FUNCTION();
    vk::ImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.image = m_image;
    viewCreateInfo.viewType = vk::ImageViewType::e2D;
    viewCreateInfo.format = m_format;
    viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.layerCount = 1;
    auto imageViewResult = device.createImageViewUnique(viewCreateInfo);
    RFCT_VULKAN_CHECK(imageViewResult.result);
    m_imageView = std::move(imageViewResult.value);
}
