#pragma once
namespace rfct {
	// A class to hold camera descriptors that are per frame in flight.
	// Camera descriptors are separate, bcs they should always use set 0, binding 0 in shader also camera can freely be reused between scenes so there is no point in moving it to scene specific render data
	// The ssbo (matrices data) is held in scene render data class
	class descriptors {
	public:
		descriptors(uint32_t size = 1);
		~descriptors();
		void bindCameraUbo(vk::Buffer ubo, uint32_t index);
		vk::DescriptorSet& getCameraDescSet(uint32_t index) { return m_cameraUboDescSet[index].get(); }
	private:
		vk::UniqueDescriptorPool m_descriptorPool;
		std::vector<vk::UniqueDescriptorSet> m_cameraUboDescSet;
	};
}