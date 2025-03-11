#include "vulkan_shader.h"
#include <fstream>
#include "renderer_p\renderer.h"
namespace rfct {

    vk::UniqueShaderModule loadShaderModule(const vk::Device& device, const std::vector<uint8_t>& source) {
        vk::ShaderModuleCreateInfo createInfo = {};
        createInfo.setCodeSize(source.size());
        createInfo.setPCode(reinterpret_cast<const uint32_t*>(source.data()));

        vk::UniqueShaderModule shaderModule = device.createShaderModuleUnique(createInfo);
        return shaderModule;
    }

    vulkanShader::vulkanShader(const std::vector<uint8_t>& shader_source)
    {
        m_shaderModule = loadShaderModule(renderer::getRen().getDevice(), shader_source);
    }

    vulkanShader::vulkanShader(const std::filesystem::path& spirvFilePath)
    {
        renderer::getRen().getAssetsManager()->loadVulkanShader(spirvFilePath.string(), this);
    }

}