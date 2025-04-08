#pragma once
namespace rfct {
	// A class to hold all dynamic descriptors that are per frame in flight.
	class descriptors {
	public:
		descriptors(uint32_t size = 1);
		~descriptors();
		void bindCameraUbo(vk::Buffer ubo, uint32_t index =0);
		vk::DescriptorSet& getCameraDescSet(uint32_t index =0) { return m_cameraUboDescSet[index].get(); }
	private:
		vk::UniqueDescriptorPool m_descriptorPool;
		std::vector<vk::UniqueDescriptorSet> m_cameraUboDescSet;
	};
}