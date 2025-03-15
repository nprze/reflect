#include "per_frame_descriptors.h"
#include "renderer_p\renderer.h"
#include "renderer_p\descriptors\camera_ubo.h"

rfct::descriptors::descriptors()
{
    // Create pool
    std::array<vk::DescriptorPoolSize, 3> poolSizes = { {
       { vk::DescriptorType::eUniformBuffer, 10 },
       { vk::DescriptorType::eStorageBuffer, 10 }
   } };

    vk::DescriptorPoolCreateInfo poolCreateInfo(
        {},             
        20,             
        poolSizes.size(),
        poolSizes.data()
    );
	m_descriptorPool = renderer::getRen().getDevice().createDescriptorPoolUnique(poolCreateInfo);
    
	// Allocate camera UBO descriptor set
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = m_descriptorPool.get();
    allocInfo.descriptorSetCount = 1;
	vk::DescriptorSetLayout descriptorSetLayout = cameraUbo::getDescriptorSetLayout();
    allocInfo.pSetLayouts = &descriptorSetLayout;

    std::vector<vk::UniqueDescriptorSet> descriptorSets = renderer::getRen().getDevice().allocateDescriptorSetsUnique(allocInfo);
	m_cameraUboDescSet = std::move(descriptorSets[0]);
}

rfct::descriptors::~descriptors()
{
}

void rfct::descriptors::bindCameraUbo(vk::Buffer ubo)
{
    if (!m_cameraUboDescSet.get()) {
		RFCT_CRITICAL("Camera UBO descriptor set is null");
    }
    vk::DescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = ubo;
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    vk::WriteDescriptorSet descriptorWrite{};
    descriptorWrite.dstSet = m_cameraUboDescSet.get();
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    renderer::getRen().getDevice().updateDescriptorSets(descriptorWrite, nullptr);

}
