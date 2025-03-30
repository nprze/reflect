#pragma once
#include "image.h"
namespace rfct {
	class bindableImage {
	public:
		bindableImage(const std::string& path);
		vk::UniqueSampler m_sampler;
		image m_Image;

	};
}