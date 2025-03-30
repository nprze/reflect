#include "bindable_image.h"
#include "renderer_p/renderer.h"
rfct::bindableImage::bindableImage(const std::string& path) :m_Image(path)
{
    // Sampler
    vk::SamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.magFilter = vk::Filter::eLinear;
    samplerCreateInfo.minFilter = vk::Filter::eLinear;
    samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;

   m_sampler = renderer::getRen().getDevice().createSamplerUnique(samplerCreateInfo);
}
