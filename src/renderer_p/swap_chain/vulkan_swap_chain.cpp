#include "vulkan_swap_chain.h"
#include "renderer_p/renderer.h"

rfct::vulkanSwapChain::vulkanSwapChain(vk::Device device, vk::PhysicalDevice physicalDevice, vk::Extent2D windowExtent)
    : m_device(device), m_physicalDevice(physicalDevice), m_windowExtent(windowExtent)
{
    createSwapChain();
	createImageViews();
}

rfct::vulkanSwapChain::~vulkanSwapChain()
{
}

void rfct::vulkanSwapChain::createSwapChain()
{
    vk::SurfaceCapabilitiesKHR capabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(renderer::getRen().getSurface());
    std::vector<vk::SurfaceFormatKHR> surfaceFormats = m_physicalDevice.getSurfaceFormatsKHR(renderer::getRen().getSurface());
    std::vector<vk::PresentModeKHR> presentModes = m_physicalDevice.getSurfacePresentModesKHR(renderer::getRen().getSurface());
    vk::SurfaceFormatKHR chosenSurfaceFormat = surfaceFormats[0];

    for (const auto& format : surfaceFormats) {
        if (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear && format.format == vk::Format::eB8G8R8A8Unorm) {
            chosenSurfaceFormat = format;
            break;
        }
    }
    m_surfaceFormat = chosenSurfaceFormat;
    vk::PresentModeKHR  chosenPresentMode = vk::PresentModeKHR::eFifo;
#ifdef WINDOWS_BUILD
    for (vk::PresentModeKHR mode : presentModes) {
        if (mode == vk::PresentModeKHR::eMailbox)  chosenPresentMode = vk::PresentModeKHR::eMailbox;
    }
#endif // WINDOWS_BUILD
    RFCT_TRACE("Choosen swap chain present mode: {0}", chosenPresentMode == vk::PresentModeKHR::eMailbox? "Mailbox": "Fifo");
    m_swapChainExtent = capabilities.currentExtent;
    vk::SwapchainCreateInfoKHR swapChainCreateInfo = {};
    swapChainCreateInfo.surface = renderer::getRen().m_surface.surface;
#ifdef WINDOWS_BUILD
    if (m_swapChain.get()!=nullptr)
    {
        swapChainCreateInfo.oldSwapchain = m_swapChain.get();
    }
#endif // WINDOWS_BUILD
    swapChainCreateInfo.minImageCount = RFCT_FRAMES_IN_FLIGHT + 1;
    swapChainCreateInfo.imageFormat = chosenSurfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = chosenSurfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = capabilities.currentExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    swapChainCreateInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
    swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapChainCreateInfo.presentMode = chosenPresentMode;
    swapChainCreateInfo.clipped = VK_TRUE;

    m_swapChain = m_device.createSwapchainKHRUnique(swapChainCreateInfo);
}

void rfct::vulkanSwapChain::recreateSwapChain()
{
    renderer::getRen().getDevice().waitIdle();
    vk::SurfaceCapabilitiesKHR capabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(renderer::getRen().getSurface());
    if (capabilities.currentExtent.width == 0 || capabilities.currentExtent.height == 0) return;
	m_device.waitIdle();
	createSwapChain();
	createImageViews();
    cleanupMSAAres();
    createMSAAres(msaaSamples);
	createFrameBuffers();
}

void rfct::vulkanSwapChain::createImageViews()
{ 
    m_swapChainImage = m_device.getSwapchainImagesKHR(m_swapChain.get());
    m_swapChainImageViews.resize(m_swapChainImage.size());

    for (size_t i = 0; i < m_swapChainImage.size(); i++) {
        vk::ImageViewCreateInfo viewCreateInfo = {};
        viewCreateInfo.image = m_swapChainImage[i];
        viewCreateInfo.viewType = vk::ImageViewType::e2D;
        viewCreateInfo.format = m_surfaceFormat.format;
        viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.layerCount = 1;

        m_swapChainImageViews[i] = m_device.createImageViewUnique(viewCreateInfo);
    }
}

void rfct::vulkanSwapChain::createFrameBuffers()
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
            frameBufferCreateInfo.renderPass = renderer::getRen().getRasterizerPipeline().getRenderPass();
            frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            frameBufferCreateInfo.pAttachments = attachments.data();
            frameBufferCreateInfo.width = m_swapChainExtent.width;
            frameBufferCreateInfo.height = m_swapChainExtent.height;
            frameBufferCreateInfo.layers = 1;
            m_frameBuffers[i] = m_device.createFramebufferUnique(frameBufferCreateInfo);
        } 
        {
            std::array<vk::ImageView, 1> attachments = {
                m_swapChainImageViews[i].get(),
            };
            vk::FramebufferCreateInfo frameBufferCreateInfo = {};
            frameBufferCreateInfo.renderPass = renderer::getRen().getUIPipeline().getRenderPass();
            frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            frameBufferCreateInfo.pAttachments = attachments.data();
            frameBufferCreateInfo.width = m_swapChainExtent.width;
            frameBufferCreateInfo.height = m_swapChainExtent.height;
            frameBufferCreateInfo.layers = 1;
            m_UIframeBuffers[i] = m_device.createFramebufferUnique(frameBufferCreateInfo);
        }
        //renderer::getRen().setObjectName(m_frameBuffers[i].get(), "swapchain framebuffer", vk::ObjectType::eFramebuffer);
	}
}
void rfct::vulkanSwapChain::createMSAAres(vk::SampleCountFlagBits msaaSamples)
{
    m_msaaColorImages.resize(m_swapChainImageViews.size());
    m_msaaImageAllocations.resize(m_swapChainImageViews.size());
    m_msaaColorImageViews.resize(m_swapChainImageViews.size());

    for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
        // Create Vulkan image
        vk::ImageCreateInfo imageInfo({}, vk::ImageType::e2D, m_surfaceFormat.format,
            { static_cast<uint32_t>(m_swapChainExtent.width), static_cast<uint32_t>(m_swapChainExtent.height), 1 }, 1, 1,
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
        viewInfo.format = m_surfaceFormat.format;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;

        m_msaaColorImageViews[i] = m_device.createImageViewUnique(viewInfo);
    }
}
void rfct::vulkanSwapChain::cleanupMSAAres()
{
    if (m_msaaColorImages[0]) {
        for (uint32_t i = 0; i < m_msaaColorImages.size(); i++) {
            vmaDestroyImage(renderer::getRen().getAllocator(), static_cast<VkImage>(m_msaaColorImages[i]), m_msaaImageAllocations[i]);
        }
    }
}
uint32_t rfct::vulkanSwapChain::acquireNextImage(const vk::Semaphore& semaphore, vk::Fence fence)
{
    vk::ResultValue<uint32_t> result = vk::ResultValue<uint32_t>(vk::Result::eSuccess, 0);

    if (framebufferResized)
    {
        recreateSwapChain();
        framebufferResized = false;
    }

    try
    {
        result = m_device.acquireNextImageKHR(m_swapChain.get(), UINT64_MAX, semaphore, fence);
    }
    catch (const vk::OutOfDateKHRError&)
    {
        recreateSwapChain();
        return -1;
    }

    if (result.result == vk::Result::eSuboptimalKHR)
    {
        RFCT_WARN("Swap chain is suboptimal, recreating...");
        recreateSwapChain();
        return -1;
    }

    if (result.result != vk::Result::eSuccess)
    {
        RFCT_CRITICAL("Failed to acquire swap chain image!");
    }

    return result.value;
}
