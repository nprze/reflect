#pragma once
#include "renderer_p/buffer/vulkan_buffer.h"
#include "assets/assets_manager.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"

namespace rfct {
	struct buildingBlockMesh {
		buildingBlockMesh(const std::string& path, const glm::vec3& color, const glm::vec2& size) {
			m_Vertices.reserve(500);
			AssetsManager::get().loadBuildingBlockMesh(path, &m_Vertices, color, size);
		};
		std::vector<Vertex> m_Vertices;
	private:
	};
	struct backgroundMesh {
		backgroundMesh(const std::string& path, const glm::vec3& color, const float zMin, const float zMax) {
			m_Vertices.reserve(500);
			AssetsManager::get().loadBackgroundMesh(path, &m_Vertices, color, zMin, zMax);
		};
		std::vector<Vertex> m_Vertices;
	private:
	};
	struct mesh {
		mesh(const std::string& path) {
			m_Vertices.reserve(500);
			AssetsManager::get().loadCharacterMesh(path, &m_Vertices, 1);
		};
		std::vector<Vertex> m_Vertices;
	private:
	};
}