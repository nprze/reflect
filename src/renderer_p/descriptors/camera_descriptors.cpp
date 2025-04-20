#include "camera_descriptors.h"
#include "renderer_p/renderer.h"
#include "renderer_p/descriptors/camera_ubo.h"

rfct::descriptors::descriptors(uint32_t size)
{
    // Create pool
    std::array<vk::DescriptorPoolSize, 1> poolSizes = { {
       { vk::DescriptorType::eUniformBuffer, size },
   } };

    vk::DescriptorPoolCreateInfo poolCreateInfo(
        vk::DescriptorPoolCreateFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet),
        size,             
        poolSizes.size(),
        poolSizes.data()
    );
	m_descriptorPool = renderer::getRen().getDevice().createDescriptorPoolUnique(poolCreateInfo);
	for (size_t i = 0; i < size; i++)
	{
        // Allocate camera UBO descriptor set
        vk::DescriptorSetAllocateInfo allocInfo{};
        allocInfo.descriptorPool = m_descriptorPool.get();
        allocInfo.descriptorSetCount = 1;
        vk::DescriptorSetLayout descriptorSetLayout = cameraUbo::getDescriptorSetLayout();
        allocInfo.pSetLayouts = &descriptorSetLayout;
		auto descriptorSets = renderer::getRen().getDevice().allocateDescriptorSetsUnique(allocInfo);
		m_cameraUboDescSet.push_back(std::move(descriptorSets[0]));
	}
}

rfct::descriptors::~descriptors()
{
}

void rfct::descriptors::bindCameraUbo(vk::Buffer ubo, uint32_t index)
{
    if (!m_cameraUboDescSet[index].get()) {
		RFCT_CRITICAL("Camera UBO descriptor set is null");
    }
    vk::DescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = ubo;
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    vk::WriteDescriptorSet descriptorWrite{};
    descriptorWrite.dstSet = m_cameraUboDescSet[index].get();
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    renderer::getRen().getDevice().updateDescriptorSets(descriptorWrite, nullptr);

}
