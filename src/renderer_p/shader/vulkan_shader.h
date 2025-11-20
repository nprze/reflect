#pragma once
namespace rfct {
    struct vulkanShader
    {
    public:
        vulkanShader(const std::string& spirvFilePath);
        ~vulkanShader() = default;
        inline vk::ShaderModule getShaderModule() { return m_shaderModule.get(); }
    private:
        vk::UniqueShaderModule m_shaderModule;
    };
} // namespace rfct