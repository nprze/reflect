#include "ray_tracer.h"
#include "renderer_p\renderer.h"

rfct::rayTracer::rayTracer()
{
	vk::PhysicalDeviceProperties2 prop2;
	prop2.pNext = &m_rtProperties;
	m_rtProperties.sType = vk::StructureType::ePhysicalDeviceRayTracingPipelinePropertiesKHR;
	renderer::ren.getDeviceWrapper().getPhysicalDevice().getProperties2(&prop2);
	RFCT_TRACE("Chosen physical device ray tracing capabilites:");
	RFCT_TRACE("max recursion depth: {0}", m_rtProperties.maxRayRecursionDepth);
	RFCT_TRACE("max shader group depth: {0}", m_rtProperties.maxShaderGroupStride);
}
