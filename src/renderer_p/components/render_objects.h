#pragma once
#include <vulkan/vulkan.hpp>
#include <string>

namespace rfct {
    class RfctShader {
    public:
        RfctShader(vk::Device device, const std::string& spirvFilePath);
        inline vk::ShaderModule getShaderModule() { return m_shaderModule.get(); }
    private:
        vk::UniqueShaderModule m_shaderModule;
    };
}