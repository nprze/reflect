#pragma once
#include "vulkan\vulkan.hpp"
namespace rfct {
	class rayTracer
	{
	public:
		rayTracer();
	private:
		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR m_rtProperties;
	};
}