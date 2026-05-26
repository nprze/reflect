#include "render_target_manager.h"
#include "assets/assets_utils.h"
#include "renderer_p/renderer.h"

namespace rfct {
    void transformImage(vk::Image im, vk::ImageLayout newLayout) {
        RFCT_PROFILE_FUNCTION();
        vk::CommandBufferAllocateInfo allocInfo(
            getAssetsCommandPool(),
            vk::CommandBufferLevel::ePrimary,
            1
        );
        vk::CommandBuffer commandBuffer = renderer::getRen().getDevice().allocateCommandBuffers(allocInfo)[0];

        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        commandBuffer.begin(beginInfo);

        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = im;
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
        vk::Fence fence = renderer::getRen().getDevice().createFence(fenceInfo);
        renderer::getRen().getDeviceWrapper().getQueueManager().submitGraphics(submitInfo, fence);
        RFCT_VULKAN_CHECK(renderer::getRen().getDevice().waitForFences(fence, VK_TRUE, UINT64_MAX));

        renderer::getRen().getDevice().freeCommandBuffers(getAssetsCommandPool(), commandBuffer);
        renderer::getRen().getDevice().destroyFence(fence);
    }

    void renderImagesManager::getSwapChainImages() {
        m_swapchainImages = renderer::getRen().getDevice().getSwapchainImagesKHR(m_swapChain.getSwapChain());
        for (uint32_t i = 0; i < RFCT_FRAMES_IN_FLIGHT + 1; i++) {
            transformImage(m_swapchainImages[i], vk::ImageLayout::ePresentSrcKHR);
        }
    }

    void renderImagesManager::createImages() {
        RFCT_PROFILE_FUNCTION();
        m_sceneImages.resize(m_swapchainImages.size());
        m_bloom1Images.resize(m_swapchainImages.size());
        m_bloom2Images.resize(m_swapchainImages.size());

        m_sceneImagesAllocations.resize(m_swapchainImages.size());
        m_bloom1ImagesAllocations.resize(m_swapchainImages.size());
        m_bloom2ImagesAllocations.resize(m_swapchainImages.size());

        for (size_t i = 0; i < m_swapchainImages.size(); i++) {
            // Create Vulkan image
            vk::ImageCreateInfo imageInfo({}, vk::ImageType::e2D, m_swapChain.m_surfaceFormat.format,
                { static_cast<uint32_t>(m_swapChain.m_swapChainExtent.width), static_cast<uint32_t>(m_swapChain.m_swapChainExtent.height), 1 }, 1, 1,
                vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
                vk::SharingMode::eExclusive);

            VmaAllocationCreateInfo imageAllocInfo{};
            imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            if (vmaCreateImage(renderer::getRen().getAllocator(), reinterpret_cast<const VkImageCreateInfo*>(&imageInfo), &imageAllocInfo,
                reinterpret_cast<VkImage*>(&m_sceneImages[i]), &m_sceneImagesAllocations[i], nullptr) != VK_SUCCESS) {
                RFCT_CRITICAL("Failed to create Vulkan image");
            }
            transformImage(m_sceneImages[i], vk::ImageLayout::eColorAttachmentOptimal);
            if (vmaCreateImage(renderer::getRen().getAllocator(), reinterpret_cast<const VkImageCreateInfo*>(&imageInfo), &imageAllocInfo,
                reinterpret_cast<VkImage*>(&m_bloom1Images[i]), &m_bloom1ImagesAllocations[i], nullptr) != VK_SUCCESS) {
                RFCT_CRITICAL("Failed to create Vulkan image");
            }
            transformImage(m_bloom1Images[i], vk::ImageLayout::eColorAttachmentOptimal);
            if (vmaCreateImage(renderer::getRen().getAllocator(), reinterpret_cast<const VkImageCreateInfo*>(&imageInfo), &imageAllocInfo,
                reinterpret_cast<VkImage*>(&m_bloom2Images[i]), &m_bloom2ImagesAllocations[i], nullptr) != VK_SUCCESS) {
                RFCT_CRITICAL("Failed to create Vulkan image");
            }
            transformImage(m_bloom2Images[i], vk::ImageLayout::eColorAttachmentOptimal);
        }
    }

    void renderImagesManager::createImageViews() {
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
                viewCreateInfo.format = m_swapChain.m_surfaceFormat.format;
                viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                viewCreateInfo.subresourceRange.levelCount = 1;
                viewCreateInfo.subresourceRange.layerCount = 1;

                m_swapChainImageViews[i] = renderer::getRen().getDevice().createImageViewUnique(viewCreateInfo);
            }
            vk::ImageViewCreateInfo viewCreateInfo = {};
            viewCreateInfo.viewType = vk::ImageViewType::e2D;
            viewCreateInfo.format = m_swapChain.m_surfaceFormat.format;
            viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            viewCreateInfo.subresourceRange.levelCount = 1;
            viewCreateInfo.subresourceRange.layerCount = 1;

            viewCreateInfo.image = m_sceneImages[i];
            m_sceneImageViews[i] = renderer::getRen().getDevice().createImageViewUnique(viewCreateInfo);
            
            viewCreateInfo.image = m_bloom1Images[i];
            m_bloom1ImageViews[i] = renderer::getRen().getDevice().createImageViewUnique(viewCreateInfo);
            
            viewCreateInfo.image = m_bloom2Images[i];
            m_bloom2ImageViews[i] = renderer::getRen().getDevice().createImageViewUnique(viewCreateInfo);
        }
    }

    void renderImagesManager::createRenderPasses() {
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

            m_UIRenderPass = renderer::getRen().getDevice().createRenderPassUnique(renderPassInfo);

            colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
            renderPassInfo.pAttachments = &colorAttachment;

            m_UIimageRenderPass = renderer::getRen().getDevice().createRenderPassUnique(renderPassInfo);
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

            m_IntermediateRenderPass = renderer::getRen().getDevice().createRenderPassUnique(renderPassInfo);
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

            m_IntermediateClearRenderPass = renderer::getRen().getDevice().createRenderPassUnique(renderPassInfo);
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

            m_presentToColorAttachment = renderer::getRen().getDevice().createRenderPassUnique(renderPassInfo);
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

    void renderImagesManager::createFrameBuffers() {
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
                frameBufferCreateInfo.width = m_swapChain.getExtent().width;
                frameBufferCreateInfo.height = m_swapChain.getExtent().height;
                frameBufferCreateInfo.layers = 1;
                m_sceneFramebuffers[i] = renderer::getRen().getDevice().createFramebufferUnique(frameBufferCreateInfo);
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
                m_swapchainFramebuffers[i] = renderer::getRen().getDevice().createFramebufferUnique(frameBufferCreateInfo);
            }
            {
                std::array<vk::ImageView, 1> attachments = {
                    m_bloom1ImageViews[i].get(),
                };
                vk::FramebufferCreateInfo frameBufferCreateInfo = {};
                frameBufferCreateInfo.renderPass = m_UIRenderPass.get();
                frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                frameBufferCreateInfo.pAttachments = attachments.data();
                frameBufferCreateInfo.width = m_swapChain.getExtent().width;
                frameBufferCreateInfo.height = m_swapChain.getExtent().height;
                frameBufferCreateInfo.layers = 1;
                m_bloom1Framebuffers[i] = renderer::getRen().getDevice().createFramebufferUnique(frameBufferCreateInfo);
            }
            {
                std::array<vk::ImageView, 1> attachments = {
                    m_bloom2ImageViews[i].get(),
                };
                vk::FramebufferCreateInfo frameBufferCreateInfo = {};
                frameBufferCreateInfo.renderPass = m_UIRenderPass.get();
                frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                frameBufferCreateInfo.pAttachments = attachments.data();
                frameBufferCreateInfo.width = m_swapChain.getExtent().width;
                frameBufferCreateInfo.height = m_swapChain.getExtent().height;
                frameBufferCreateInfo.layers = 1;
                m_bloom2Framebuffers[i] = renderer::getRen().getDevice().createFramebufferUnique(frameBufferCreateInfo);
            }
        }
    }

    void renderImagesManager::createMSAAres(vk::SampleCountFlagBits msaaSamples) {
        RFCT_PROFILE_FUNCTION();
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

    void renderImagesManager::cleanupMSAAres() {
        RFCT_PROFILE_FUNCTION();
        if (m_msaaColorImages.size()) {
            for (uint32_t i = 0; i < m_msaaColorImages.size(); i++) {
                vmaDestroyImage(renderer::getRen().getAllocator(), static_cast<VkImage>(m_msaaColorImages[i]), m_msaaImageAllocations[i]);
            }
        }
    }

    void renderImagesManager::cleanupImages() {
        RFCT_PROFILE_FUNCTION();
        for (uint32_t i = 0; i < m_bloom1ImagesAllocations.size(); i++) {
            vmaDestroyImage(renderer::getRen().getAllocator(), static_cast<VkImage>(m_bloom1Images[i]), m_bloom1ImagesAllocations[i]);
            vmaDestroyImage(renderer::getRen().getAllocator(), static_cast<VkImage>(m_bloom2Images[i]), m_bloom2ImagesAllocations[i]);
            vmaDestroyImage(renderer::getRen().getAllocator(), static_cast<VkImage>(m_sceneImages[i]), m_sceneImagesAllocations[i]);
        }
    }

    uint32_t renderImagesManager::acquireNextImage(const vk::Semaphore& sem, vk::Fence fence) {
        RFCT_PROFILE_FUNCTION();
        if (m_swapChain.framebufferResized) {
            m_swapChain.recreateSwapChain();
            createResources();
            renderer::getRen().getBloomRes().onSwapchainExtentChanged();
            m_swapChain.framebufferResized = false;
        }
        uint32_t res = m_swapChain.acquireNextImage(sem, fence);
        if (res == -1) {
            createResources();
            renderer::getRen().getBloomRes().onSwapchainExtentChanged();
        }
        return res;
    }

    renderImagesManager::renderImagesManager() 
        : m_swapChain() {
        createRenderPasses();
        createResources();
    }

    renderImagesManager::~renderImagesManager() {
        cleanupMSAAres();
        cleanupImages();
    }

    void renderImagesManager::createResources() {
        getSwapChainImages();
        cleanupImages();
        
        createImages();
        createImageViews();

        cleanupMSAAres();
        createMSAAres(msaaSamples);

        createFrameBuffers();
    }
}