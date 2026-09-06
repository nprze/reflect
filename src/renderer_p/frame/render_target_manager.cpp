#include "render_target_manager.h"
#include "assets/assets_utils.h"
#include "renderer_p/components/renderer_components.h"

void TransformImage(rfct::RfctDevice& deviceWrapper, rfct::RfctQueue& queue, vk::Image im, vk::ImageLayout newLayout) {
}

void rfct::renderImagesManager::CreateImages(rfct::RfctDevice& deviceWrapper, rfct::RfctQueue& queueWrapper,
    RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper) {
    RFCT_PROFILE_FUNCTION();
    m_sceneImages.resize(m_swapchainImages.size());
    m_bloom1Images.resize(m_swapchainImages.size());
    m_bloom2Images.resize(m_swapchainImages.size());

    m_sceneImagesAllocations.resize(m_swapchainImages.size());
    m_bloom1ImagesAllocations.resize(m_swapchainImages.size());
    m_bloom2ImagesAllocations.resize(m_swapchainImages.size());

    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
        // Create Vulkan image
        vk::ImageCreateInfo imageInfo({}, vk::ImageType::e2D, swapChainWrapper.GetSurfaceFormat().format,
            { static_cast<uint32_t>(swapChainWrapper.GetExtent().width), static_cast<uint32_t>(swapChainWrapper.GetExtent().height), 1 }, 1, 1,
            vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
            vk::SharingMode::eExclusive);

        VmaAllocationCreateInfo imageAllocInfo{};
        imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        if (vmaCreateImage(allocatorWrapper.GetAllocator(), reinterpret_cast<const VkImageCreateInfo*>(&imageInfo), &imageAllocInfo,
            reinterpret_cast<VkImage*>(&m_sceneImages[i]), &m_sceneImagesAllocations[i], nullptr) != VK_SUCCESS) {
            RFCT_CRITICAL("Failed to create Vulkan image");
        }
        TransformImage(deviceWrapper, queueWrapper, m_sceneImages[i], vk::ImageLayout::eColorAttachmentOptimal);
        if (vmaCreateImage(allocatorWrapper.GetAllocator(), reinterpret_cast<const VkImageCreateInfo*>(&imageInfo), &imageAllocInfo,
            reinterpret_cast<VkImage*>(&m_bloom1Images[i]), &m_bloom1ImagesAllocations[i], nullptr) != VK_SUCCESS) {
            RFCT_CRITICAL("Failed to create Vulkan image");
        }
        TransformImage(deviceWrapper, queueWrapper, m_bloom1Images[i], vk::ImageLayout::eColorAttachmentOptimal);
        if (vmaCreateImage(allocatorWrapper.GetAllocator(), reinterpret_cast<const VkImageCreateInfo*>(&imageInfo), &imageAllocInfo,
            reinterpret_cast<VkImage*>(&m_bloom2Images[i]), &m_bloom2ImagesAllocations[i], nullptr) != VK_SUCCESS) {
            RFCT_CRITICAL("Failed to create Vulkan image");
        }
        TransformImage(deviceWrapper, queueWrapper, m_bloom2Images[i], vk::ImageLayout::eColorAttachmentOptimal);
    }
}

void rfct::renderImagesManager::CreateImageViews(RfctSwapChain& swapChainWrapper, vk::Device device) {
    RFCT_PROFILE_FUNCTION();
    m_swapChainImageViews.resize(m_swapchainImages.size());
    m_sceneImageViews.resize(m_sceneImages.size());
    m_bloom1ImageViews.resize(m_bloom1Images.size());
    m_bloom2ImageViews.resize(m_bloom2Images.size());

    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
        {
            vk::ImageViewCreateInfo viewCreateInfo = {};
            viewCreateInfo.image = m_swapchainImages[i];
            viewCreateInfo.viewType = vk::ImageViewType::e2D;
            viewCreateInfo.format = swapChainWrapper.GetSurfaceFormat().format;
            viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            viewCreateInfo.subresourceRange.levelCount = 1;
            viewCreateInfo.subresourceRange.layerCount = 1;

			auto imageViewResult = device.createImageViewUnique(viewCreateInfo);
			RFCT_VULKAN_CHECK(imageViewResult.result);
            m_swapChainImageViews[i] = std::move(imageViewResult.value);
        }
        vk::ImageViewCreateInfo viewCreateInfo = {};
        viewCreateInfo.viewType = vk::ImageViewType::e2D;
        viewCreateInfo.format = swapChainWrapper.GetSurfaceFormat().format;
        viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.layerCount = 1;

        viewCreateInfo.image = m_sceneImages[i];
        auto sceneImageViewResult = device.createImageViewUnique(viewCreateInfo);
        RFCT_VULKAN_CHECK(sceneImageViewResult.result);
        m_sceneImageViews[i] = std::move(sceneImageViewResult.value);
            
        viewCreateInfo.image = m_bloom1Images[i];
        auto bloom1ImageViewResult = device.createImageViewUnique(viewCreateInfo);
        RFCT_VULKAN_CHECK(bloom1ImageViewResult.result);
        m_bloom1ImageViews[i] = std::move(bloom1ImageViewResult.value);
            
        viewCreateInfo.image = m_bloom2Images[i];
        auto bloom2ImageViewResult = device.createImageViewUnique(viewCreateInfo);
        RFCT_VULKAN_CHECK(bloom2ImageViewResult.result);
        m_bloom2ImageViews[i] = std::move(bloom2ImageViewResult.value);
    }
}

void rfct::renderImagesManager::CreateRenderPasses(vk::Device device, vk::SampleCountFlagBits msaaSamples) {
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

void rfct::renderImagesManager::CreateFrameBuffers(RfctSwapChain& swapChainWrapper, vk::Device device) {
    RFCT_PROFILE_FUNCTION();
    m_swapchainFramebuffers.resize(m_swapChainImageViews.size());
    m_sceneFramebuffers.resize(m_swapChainImageViews.size());
    m_bloom1Framebuffers.resize(m_swapChainImageViews.size());
    m_bloom2Framebuffers.resize(m_swapChainImageViews.size());
    for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
        {
            std::array<vk::ImageView, 2> attachments = {
                m_msaaColorImageViews[i].get(),
                m_sceneImageViews[i].get(),
            };
            vk::FramebufferCreateInfo frameBufferCreateInfo = {};
            frameBufferCreateInfo.renderPass = m_sceneRenderPass.get();
            frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            frameBufferCreateInfo.pAttachments = attachments.data();
            frameBufferCreateInfo.width = swapChainWrapper.GetExtent().width;
            frameBufferCreateInfo.height = swapChainWrapper.GetExtent().height;
            frameBufferCreateInfo.layers = 1;
            m_sceneFramebuffers[i] = device.createFramebufferUnique(frameBufferCreateInfo).value;
        }
        {
            std::array<vk::ImageView, 1> attachments = {
                m_swapChainImageViews[i].get(),
            };
            vk::FramebufferCreateInfo frameBufferCreateInfo = {};
            frameBufferCreateInfo.renderPass = m_UIRenderPass.get();
            frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            frameBufferCreateInfo.pAttachments = attachments.data();
            frameBufferCreateInfo.width = swapChainWrapper.GetExtent().width;
            frameBufferCreateInfo.height = swapChainWrapper.GetExtent().height;
            frameBufferCreateInfo.layers = 1;
            m_swapchainFramebuffers[i] = device.createFramebufferUnique(frameBufferCreateInfo).value;
        }
        {
            std::array<vk::ImageView, 1> attachments = {
                m_bloom1ImageViews[i].get(),
            };
            vk::FramebufferCreateInfo frameBufferCreateInfo = {};
            frameBufferCreateInfo.renderPass = m_UIRenderPass.get();
            frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            frameBufferCreateInfo.pAttachments = attachments.data();
            frameBufferCreateInfo.width = swapChainWrapper.GetExtent().width;
            frameBufferCreateInfo.height = swapChainWrapper.GetExtent().height;
            frameBufferCreateInfo.layers = 1;
            m_bloom1Framebuffers[i] = device.createFramebufferUnique(frameBufferCreateInfo).value;
        }
        {
            std::array<vk::ImageView, 1> attachments = {
                m_bloom2ImageViews[i].get(),
            };
            vk::FramebufferCreateInfo frameBufferCreateInfo = {};
            frameBufferCreateInfo.renderPass = m_UIRenderPass.get();
            frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            frameBufferCreateInfo.pAttachments = attachments.data();
            frameBufferCreateInfo.width = swapChainWrapper.GetExtent().width;
            frameBufferCreateInfo.height = swapChainWrapper.GetExtent().height;
            frameBufferCreateInfo.layers = 1;
            m_bloom2Framebuffers[i] = device.createFramebufferUnique(frameBufferCreateInfo).value;
        }
    }
}

void rfct::renderImagesManager::CreateMSAAres(RfctSwapChain& swapChainWrapper, RfctVulkanMemAllocator& allocatorWrapper, vk::Device device, vk::SampleCountFlagBits msaaSamples) {
    RFCT_PROFILE_FUNCTION();
    m_msaaColorImages.resize(m_swapChainImageViews.size());
    m_msaaImageAllocations.resize(m_swapChainImageViews.size());
    m_msaaColorImageViews.resize(m_swapChainImageViews.size());

    for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
        // Create Vulkan image
        vk::ImageCreateInfo imageInfo({}, vk::ImageType::e2D, swapChainWrapper.GetSurfaceFormat().format,
            { static_cast<uint32_t>(swapChainWrapper.GetExtent().width), static_cast<uint32_t>(swapChainWrapper.GetExtent().height), 1 }, 1, 1,
            msaaSamples, vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
            vk::SharingMode::eExclusive);

        VmaAllocationCreateInfo imageAllocInfo{};
        imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        if (vmaCreateImage(allocatorWrapper.GetAllocator(), reinterpret_cast<const VkImageCreateInfo*>(&imageInfo), &imageAllocInfo,
            reinterpret_cast<VkImage*>(&m_msaaColorImages[i]), &m_msaaImageAllocations[i], nullptr) != VK_SUCCESS) {
            RFCT_CRITICAL("Failed to create Vulkan image");
        }

        vk::ImageViewCreateInfo viewInfo = {};
        viewInfo.image = m_msaaColorImages[i];
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = swapChainWrapper.GetSurfaceFormat().format;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;

		auto imageViewCreateResult = device.createImageViewUnique(viewInfo);
		RFCT_VULKAN_CHECK(imageViewCreateResult.result);
        m_msaaColorImageViews[i] = std::move(imageViewCreateResult.value);
    }
}

void rfct::renderImagesManager::CleanupMSAAres(RfctVulkanMemAllocator& allocatorWrapper) {
    RFCT_PROFILE_FUNCTION();
    if (m_msaaColorImages.size()) {
        for (uint32_t i = 0; i < m_msaaColorImages.size(); i++) {
            vmaDestroyImage(allocatorWrapper.GetAllocator(), static_cast<VkImage>(m_msaaColorImages[i]), m_msaaImageAllocations[i]);
        }
    }
}

void rfct::renderImagesManager::CleanupImages(RfctVulkanMemAllocator& allocatorWrapper) {
    RFCT_PROFILE_FUNCTION();
    for (uint32_t i = 0; i < m_bloom1ImagesAllocations.size(); i++) {
        vmaDestroyImage(allocatorWrapper.GetAllocator(), static_cast<VkImage>(m_bloom1Images[i]), m_bloom1ImagesAllocations[i]);
        vmaDestroyImage(allocatorWrapper.GetAllocator(), static_cast<VkImage>(m_bloom2Images[i]), m_bloom2ImagesAllocations[i]);
        vmaDestroyImage(allocatorWrapper.GetAllocator(), static_cast<VkImage>(m_sceneImages[i]), m_sceneImagesAllocations[i]);
    }
}

rfct::renderImagesManager::renderImagesManager(rfct::RfctDevice& deviceWrapper, rfct::RfctQueue& queueWrapper,
    RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper) {
    CreateRenderPasses(deviceWrapper.GetDevice());
    CreateResources(deviceWrapper, queueWrapper, allocatorWrapper, swapChainWrapper);
}

rfct::renderImagesManager::~renderImagesManager() {
    // TODO: Do the actual cleanup before destructor
    /* CleanupMSAAres();
    CleanupImages();*/  
}

void rfct::renderImagesManager::CreateResources(rfct::RfctDevice& deviceWrapper, rfct::RfctQueue& queueWrapper,
    RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper) {
	auto swapChainImagesResult = deviceWrapper.GetDevice().getSwapchainImagesKHR(swapChainWrapper.GetSwapChain());
	RFCT_VULKAN_CHECK(swapChainImagesResult.result);
    m_swapchainImages = swapChainImagesResult.value;
    for (uint32_t i = 0; i < RFCT_FRAMES_IN_FLIGHT + 1; i++) {
        TransformImage(deviceWrapper, queueWrapper, m_swapchainImages[i], vk::ImageLayout::ePresentSrcKHR);
    }
    CleanupImages(allocatorWrapper);
        
    CreateImages(deviceWrapper, queueWrapper, allocatorWrapper, swapChainWrapper);
    CreateImageViews(swapChainWrapper, deviceWrapper.GetDevice());

    CleanupMSAAres(allocatorWrapper);
    CreateMSAAres(swapChainWrapper, allocatorWrapper, deviceWrapper.GetDevice());
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
    commandBuffer.begin(beginInfo);

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

    commandBuffer.end();

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

void rfct::RfctRenderImage::AllocateImage(const RfctRenderImage::RfctRenderImageSpec& spec, RfctDevice& deviceWrapper, RfctQueue& queueWrapper, RfctVulkanMemAllocator& allocatorWrapper) {
    RFCT_PROFILE_FUNCTION();
	m_format = spec.dafaultFormat;
	m_extent = spec.extent;
    // Create Vulkan image
    vk::ImageCreateInfo imageInfo({}, vk::ImageType::e2D, m_format,
        { static_cast<uint32_t>(m_extent.width), static_cast<uint32_t>(m_extent.height), 1 }, 1, 1,
        vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
        vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo imageAllocInfo{};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(allocatorWrapper.GetAllocator(), reinterpret_cast<const VkImageCreateInfo*>(&imageInfo), &imageAllocInfo,
        reinterpret_cast<VkImage*>(&m_image), &m_imageAllocation, nullptr) != VK_SUCCESS) {
        RFCT_CRITICAL("Failed to create Vulkan image");
    }
	TransformLayoutSync(spec.dafaultLayout, deviceWrapper, queueWrapper);
}

void rfct::RfctRenderImage::CreateImageView(RfctSwapChain& swapChainWrapper, vk::Device device) {
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
