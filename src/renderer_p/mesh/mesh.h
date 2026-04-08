#pragma once
#include "renderer_p/buffer/vulkan_buffer.h"
#include "assets/mesh_load.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"

namespace rfct {
	struct buildingBlockMesh {
		buildingBlockMesh(const std::string& path, const glm::vec3& color, const glm::vec2& size) {
			m_Vertices.reserve(500);
			loadBuildingBlockMesh(path, &m_Vertices, color, size);
		};
		std::vector<Vertex> m_Vertices;
	};
	struct backgroundMesh {
		backgroundMesh(const std::string& path, const glm::vec3& color, const float zMin, const float zMax) {
			m_Vertices.reserve(500);
			loadBackgroundMesh(path, &m_Vertices, color, zMin, zMax);
		};
		std::vector<Vertex> m_Vertices;
	};
	struct mesh {
		mesh(const std::string& path) {
			m_Vertices.reserve(500);
			loadCharacterMesh(path, &m_Vertices, 1);
		};
		std::vector<Vertex> m_Vertices;
	};
}