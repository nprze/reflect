#pragma once
#include "renderer_p/buffer/vulkan_buffer.h"
#include "assets/assets_manager.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"

namespace rfct {
	struct mesh {
		mesh(const std::string& path, const glm::vec3& color) { 
			m_Vertices.reserve(500);
			AssetsManager::get().loadMesh(path, this, color);
		};
		std::vector<Vertex> m_Vertices;
	private:
	};
}