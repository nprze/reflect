#pragma once
namespace rfct {
    class vulkanShader
    {
    public:
        vulkanShader(const std::string& spirvFilePath);
        ~vulkanShader() = default;
        inline vk::ShaderModule getShaderModule() { return m_shaderModule.get(); }
    private:
        vk::UniqueShaderModule m_shaderModule;
        friend class AssetsManager;
    };
} // namespace rfct