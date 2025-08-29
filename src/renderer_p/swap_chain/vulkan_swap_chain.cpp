#include "vulkan_swap_chain.h"
#include "renderer_p/renderer.h"
#include "world_p/world.h"

rfct::vulkanSwapChain::vulkanSwapChain()
{
    createSwapChain();
}

rfct::vulkanSwapChain::~vulkanSwapChain()
{
}

void rfct::vulkanSwapChain::createSwapChain()
{
    vk::SurfaceCapabilitiesKHR capabilities = renderer::getRen().getDeviceWrapper().getPhysicalDevice().getSurfaceCapabilitiesKHR(renderer::getRen().getSurface());
    std::vector<vk::SurfaceFormatKHR> surfaceFormats = renderer::getRen().getDeviceWrapper().getPhysicalDevice().getSurfaceFormatsKHR(renderer::getRen().getSurface());
    std::vector<vk::PresentModeKHR> presentModes = renderer::getRen().getDeviceWrapper().getPhysicalDevice().getSurfacePresentModesKHR(renderer::getRen().getSurface());
    vk::SurfaceFormatKHR chosenSurfaceFormat = surfaceFormats[0];
/*
    for (const auto& format : surfaceFormats) {
        if (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear && format.format == vk::Format::eB8G8R8A8Unorm) {
            chosenSurfaceFormat = format;
            break;
        }
    }*/
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

    if (capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eOpaque) {
        swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    }
#ifdef ANDROID_BUILD
    else if (capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit) {
        swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eInherit;
    }
    else {
        // Pick any available supported option
        swapChainCreateInfo.compositeAlpha = static_cast<vk::CompositeAlphaFlagBitsKHR>(
            __builtin_ctz(static_cast<uint32_t>(capabilities.supportedCompositeAlpha))
            );
    }
    vk::SurfaceTransformFlagBitsKHR transform = capabilities.currentTransform;
    switch (transform) {
        case vk::SurfaceTransformFlagBitsKHR::eRotate90:
            world::getWorld().addScreenTransform(90);
            break;
        case vk::SurfaceTransformFlagBitsKHR::eRotate180:
            world::getWorld().addScreenTransform(180);
            break;
        case vk::SurfaceTransformFlagBitsKHR::eRotate270:
            world::getWorld().addScreenTransform(270);
            break;
        default:
            break;
    }
#endif
    swapChainCreateInfo.minImageCount = RFCT_FRAMES_IN_FLIGHT + 1;
    swapChainCreateInfo.imageFormat = chosenSurfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = chosenSurfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = capabilities.currentExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
    swapChainCreateInfo.preTransform = capabilities.currentTransform;
    swapChainCreateInfo.presentMode = chosenPresentMode;
    swapChainCreateInfo.clipped = VK_TRUE;

    m_swapChain = renderer::getRen().getDevice().createSwapchainKHRUnique(swapChainCreateInfo);
}

void rfct::vulkanSwapChain::recreateSwapChain()
{
    renderer::getRen().getDevice().waitIdle();
#ifdef ANDROID_BUILD
    renderer::getRen().getDevice().destroySwapchainKHR(m_swapChain.get());
    *m_swapChain = nullptr;
#endif
    vk::SurfaceCapabilitiesKHR capabilities = renderer::getRen().getDeviceWrapper().getPhysicalDevice().getSurfaceCapabilitiesKHR(renderer::getRen().getSurface());
    if (capabilities.currentExtent.width == 0 || capabilities.currentExtent.height == 0) return;
    createSwapChain();
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
        result = renderer::getRen().getDevice().acquireNextImageKHR(m_swapChain.get(), UINT64_MAX, semaphore, fence);
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
