#pragma once
#include "image.h"
namespace rfct {
	class bindableImage {
	public:
		bindableImage(const std::string& path);
		~bindableImage();
		std::string name;
		image m_Image;
		vk::UniqueSampler m_sampler;
	};
}