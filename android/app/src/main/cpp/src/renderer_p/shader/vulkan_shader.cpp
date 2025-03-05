#include "vulkan_shader.h"
#include <fstream>
#include "renderer_p\renderer.h"
#include <android/log.h>
#include <unistd.h>
namespace rfct {

    vk::UniqueShaderModule loadShaderModule(const vk::Device& device, const std::vector<uint8_t>& source) {
        /*std::ifstream file(filename, std::ios::binary | std::ios::ate);
        __android_log_print(ANDROID_LOG_INFO, "Reflect", "%s", filename.c_str());
        char cwd[1024];  // Buffer to hold the directory path
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            __android_log_print(ANDROID_LOG_INFO, "Current Working Dir", "%s", cwd);
        } else {
            __android_log_print(ANDROID_LOG_ERROR, "Error", "Failed to get current working directory");
        }
        if (!file.is_open()) {
            int errorCode = errno;
            std::string errorMsg = strerror(errorCode);  // Get the error string corresponding to errno

            __android_log_print(ANDROID_LOG_INFO, "Fail reason", "%s", errorMsg.c_str());
			RFCT_CRITICAL("Failed to open shader file: {}", filename);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        file.seekg(0);

        std::vector<char> buffer(fileSize);
        file.read(buffer.data(), fileSize);
        file.close();
        */
        vk::ShaderModuleCreateInfo createInfo = {};
        createInfo.setCodeSize(source.size());
        createInfo.setPCode(reinterpret_cast<const uint32_t*>(source.data()));

        vk::UniqueShaderModule shaderModule = device.createShaderModuleUnique(createInfo);
        return shaderModule;
    }

    vulkanShader::vulkanShader( const std::vector<uint8_t>& shader_source)
    {
		m_shaderModule = loadShaderModule(renderer::getRen().getDevice(), shader_source);
    }

}