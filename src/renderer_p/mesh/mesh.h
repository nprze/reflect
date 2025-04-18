#pragma once
#include "renderer_p\buffer\vulkan_buffer.h"
namespace rfct {
	struct mesh {
		mesh() { m_Vertices.reserve(500); };
		std::vector<Vertex> m_Vertices;
	private:
	};
}