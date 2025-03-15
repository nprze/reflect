#pragma once
namespace rfct {
	// A class to hold all dynamic descriptors that are per frame in flight.
	class descriptors {
	public:
		descriptors();
		~descriptors();
		void bindCameraUbo(vk::Buffer ubo);
	private:
		vk::UniqueDescriptorPool m_descriptorPool;
		vk::UniqueDescriptorSet m_cameraUboDescSet;
		friend class frameData;
	};
}