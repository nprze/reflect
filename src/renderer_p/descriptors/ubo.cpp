#include "ubo.h"
#include "vma/vk_mem_alloc.h"
#include "renderer_p/renderer.h"

vk::DescriptorSetLayout uboDescriptorSetLayout;

rfct::ubo::ubo()
    : m_buffer("uniform buffer", sizeof(uboData), vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU),
    m_data() {
	m_mappedBuffer = m_buffer.Map();
}

rfct::ubo::~ubo() { 
	m_buffer.Unmap(); 
}

void rfct::ubo::updateUboData(glm::mat4 vp, float globalTime) {
	RFCT_PROFILE_FUNCTION();
	memcpy(m_mappedBuffer, &vp, sizeof(glm::mat4));
	float value = globalTime;
	memcpy((void*)((glm::mat4*)m_mappedBuffer + 1), &value, sizeof(float));
}

vk::DescriptorSetLayout rfct::ubo::getDescriptorSetLayout() {
    if (uboDescriptorSetLayout) {
        return uboDescriptorSetLayout;
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

    uboDescriptorSetLayout = renderer::getRen().getDevice().createDescriptorSetLayout(layoutCreateInfo);
	return uboDescriptorSetLayout;
}

void rfct::ubo::destroyDescriptorSetLayout() {
    renderer::getRen().getDevice().destroyDescriptorSetLayout(uboDescriptorSetLayout);
}
