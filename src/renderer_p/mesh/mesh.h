#pragma once
#include "renderer_p/buffer/vulkan_buffer.h"
#include "assets/assets_manager.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"

namespace rfct {
	struct buildingBlockMesh {
		buildingBlockMesh(const std::string& path, const glm::vec3& color) {
			m_Vertices.reserve(500);
			AssetsManager::get().loadBuildingBlockMesh(path, &m_Vertices, color);
		};
		std::vector<Vertex> m_Vertices;
	private:
	};
	struct mesh {
		mesh(const std::string& path) {
			m_Vertices.reserve(500);
			AssetsManager::get().loadCharacterMesh(path, &m_Vertices);
		};
		std::vector<Vertex> m_Vertices;
	private:
	};
}