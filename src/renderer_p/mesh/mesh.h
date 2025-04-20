#pragma once
#include "renderer_p/buffer/vulkan_buffer.h"
#include "assets/assets_manager.h"
namespace rfct {
	struct mesh {
		mesh(const std::string& path) { m_Vertices.reserve(500);
		AssetsManager::get().loadMesh(path, this);
		};
		std::vector<Vertex> m_Vertices;
	private:
	};
}