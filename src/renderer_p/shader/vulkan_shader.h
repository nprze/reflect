#pragma once
#include <filesystem>
#include <vulkan/vulkan.hpp>
namespace rfct {
	class vulkanShader
	{
	public:
		vulkanShader(const std::vector<uint8_t>& shader_source);
		vulkanShader(const std::filesystem::path& spirvFilePath);
		~vulkanShader() = default;
		inline vk::ShaderModule getShaderModule() { return m_shaderModule.get(); }
	private:
		vk::UniqueShaderModule m_shaderModule;
	};
} // namespace rfct