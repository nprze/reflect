#include "vulkan_shader.h"
#include <fstream>
#include "renderer_p\renderer.h"
namespace rfct {

    vk::UniqueShaderModule loadShaderModule(const vk::Device& device, const std::string& filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
			RFCT_CRITICAL("Failed to open shader file: {}", filename);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        file.seekg(0);

        std::vector<char> buffer(fileSize);
        file.read(buffer.data(), fileSize);
        file.close();

        vk::ShaderModuleCreateInfo createInfo = {};
        createInfo.setCodeSize(buffer.size());
        createInfo.setPCode(reinterpret_cast<const uint32_t*>(buffer.data()));

        vk::UniqueShaderModule shaderModule = device.createShaderModuleUnique(createInfo);
        return shaderModule;
    }

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
		m_shaderModule = loadShaderModule(renderer::getRen().getDevice(), spirvFilePath.string());
    }

}