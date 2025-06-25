#include "render_target_manager.h"
#include "renderer_p/renderer.h"
namespace rfct {

    void renderImagesManager::createImageViews()
    {
        m_swapChainImageViews.resize(m_swapChainImages.size());

        for (size_t i = 0; i < m_swapChainImages.size(); i++) {
            vk::ImageViewCreateInfo viewCreateInfo = {};
            viewCreateInfo.image = m_swapChainImages[i];
            viewCreateInfo.viewType = vk::ImageViewType::e2D;
            viewCreateInfo.format = m_swapChain.m_surfaceFormat.format;
            viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            viewCreateInfo.subresourceRange.levelCount = 1;
            viewCreateInfo.subresourceRange.layerCount = 1;

            m_swapChainImageViews[i] = renderer::getRen().getDevice().createImageViewUnique(viewCreateInfo);
        }
    }


    void renderImagesManager::createRenderPasses()
    {
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

            m_UIRenderPass = renderer::getRen().getDevice().createRenderPassUnique(renderPassInfo);
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

            m_sceneRenderPass = renderer::getRen().getDevice().createRenderPassUnique(renderPassInfo);
        }
    }


    void renderImagesManager::createFrameBuffers()
    {
        m_frameBuffers.resize(m_swapChainImageViews.size());
        m_UIframeBuffers.resize(m_swapChainImageViews.size());
        for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
            {
                std::array<vk::ImageView, 2> attachments = {
                    m_msaaColorImageViews[i].get(),
                    m_swapChainImageViews[i].get(),
                };
                vk::FramebufferCreateInfo frameBufferCreateInfo = {};
                frameBufferCreateInfo.renderPass = m_sceneRenderPass.get();
                frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                frameBufferCreateInfo.pAttachments = attachments.data();
                frameBufferCreateInfo.width = m_swapChain.getExtent().width;
                frameBufferCreateInfo.height = m_swapChain.getExtent().height;
                frameBufferCreateInfo.layers = 1;
                m_frameBuffers[i] = renderer::getRen().getDevice().createFramebufferUnique(frameBufferCreateInfo);
            }
            {
                std::array<vk::ImageView, 1> attachments = {
                    m_swapChainImageViews[i].get(),
                };
                vk::FramebufferCreateInfo frameBufferCreateInfo = {};
                frameBufferCreateInfo.renderPass = m_UIRenderPass.get();
                frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                frameBufferCreateInfo.pAttachments = attachments.data();
                frameBufferCreateInfo.width = m_swapChain.getExtent().width;
                frameBufferCreateInfo.height = m_swapChain.getExtent().height;
                frameBufferCreateInfo.layers = 1;
                m_UIframeBuffers[i] = renderer::getRen().getDevice().createFramebufferUnique(frameBufferCreateInfo);
            }
        }
    }
    void renderImagesManager::createMSAAres(vk::SampleCountFlagBits msaaSamples)
    {
        m_msaaColorImages.resize(m_swapChainImageViews.size());
        m_msaaImageAllocations.resize(m_swapChainImageViews.size());
        m_msaaColorImageViews.resize(m_swapChainImageViews.size());

        for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
            // Create Vulkan image
            vk::ImageCreateInfo imageInfo({}, vk::ImageType::e2D, m_swapChain.m_surfaceFormat.format,
                { static_cast<uint32_t>(m_swapChain.m_swapChainExtent.width), static_cast<uint32_t>(m_swapChain.m_swapChainExtent.height), 1 }, 1, 1,
                msaaSamples, vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
                vk::SharingMode::eExclusive);

            VmaAllocationCreateInfo imageAllocInfo{};
            imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            if (vmaCreateImage(renderer::getRen().getAllocator(), reinterpret_cast<const VkImageCreateInfo*>(&imageInfo), &imageAllocInfo,
                reinterpret_cast<VkImage*>(&m_msaaColorImages[i]), &m_msaaImageAllocations[i], nullptr) != VK_SUCCESS) {
                RFCT_CRITICAL("Failed to create Vulkan image");
            }

            vk::ImageViewCreateInfo viewInfo = {};
            viewInfo.image = m_msaaColorImages[i];
            viewInfo.viewType = vk::ImageViewType::e2D;
            viewInfo.format = m_swapChain.m_surfaceFormat.format;
            viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.layerCount = 1;

            m_msaaColorImageViews[i] = renderer::getRen().getDevice().createImageViewUnique(viewInfo);
        }
    }
    void renderImagesManager::cleanupMSAAres()
    {
        if (m_msaaColorImages.size()) {
            for (uint32_t i = 0; i < m_msaaColorImages.size(); i++) {
                vmaDestroyImage(renderer::getRen().getAllocator(), static_cast<VkImage>(m_msaaColorImages[i]), m_msaaImageAllocations[i]);
            }
        }
    }

    void renderImagesManager::getSwapChainImages()
    {
        m_swapChainImages = renderer::getRen().getDevice().getSwapchainImagesKHR(m_swapChain.getSwapChain());
    }




    uint32_t renderImagesManager::acquireNextImage(const vk::Semaphore& sem, vk::Fence fence)
    {
        if (m_swapChain.framebufferResized) {
            createResources();
        }
        return m_swapChain.acquireNextImage(sem, fence);
    }

    renderImagesManager::renderImagesManager() :m_swapChain()
    {
        createRenderPasses();
        createResources();
    }
    renderImagesManager::~renderImagesManager()
    {
        cleanupMSAAres();
    }
    void renderImagesManager::createResources()
    {
        
        getSwapChainImages();
        createImageViews();
        cleanupMSAAres();
        createMSAAres(msaaSamples);
        createFrameBuffers();
    }
}