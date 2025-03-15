#include "camera_ubo.h"
#include "vma\vk_mem_alloc.h"
#include "renderer_p\renderer.h"

vk::DescriptorSetLayout rfct::cameraUbo::m_descriptorSetLayout;

rfct::cameraUbo::cameraUbo():m_buffer(sizeof(uniformBufferObject), vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU){
	mappedData = m_buffer.Map();
}

rfct::cameraUbo::~cameraUbo() { 
	m_buffer.Unmap(); 
}

void rfct::cameraUbo::updateViewProj(glm::mat4 vp)
{
	memcpy(mappedData, &vp, sizeof(uniformBufferObject));
}

vk::DescriptorSetLayout rfct::cameraUbo::getDescriptorSetLayout()
{
    if (m_descriptorSetLayout)
    {
        return m_descriptorSetLayout;
    }
    vk::DescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
    layoutBinding.pImmutableSamplers = nullptr;

    vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.bindingCount = 1;
    layoutCreateInfo.pBindings = &layoutBinding;

    m_descriptorSetLayout = renderer::getRen().getDevice().createDescriptorSetLayout(layoutCreateInfo);
	return m_descriptorSetLayout;
}
