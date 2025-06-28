#include "scene_loader.h"
#include "assets/scene_serialize_data.h"
#include <glm/glm.hpp>
#include <unordered_set>

bool isInsideRect(const glm::vec3& point, const glm::vec2& rectMin, const glm::vec2& rectMax) {
	return (point.x >= rectMin.x && point.x <= rectMax.x) &&
		(point.y >= rectMin.y && point.y <= rectMax.y);
}

void rfct::cutoffMesh(buildingBlockMesh& meshOut, int cutoff, int width, int height)
{
	glm::vec2 min_top = { 0,0 };
	glm::vec2 max_top = { 0,0 };

	glm::vec2 min_bottom = { 0,0 };
	glm::vec2 max_bottom = { 0,0 };

	glm::vec2 min_right = { 0,0 };
	glm::vec2 max_right = { 0,0 };

	glm::vec2 min_left = { 0,0 };
	glm::vec2 max_left = { 0,0 };

	bool top = (cutoff & cutoffValues::top) != 0;
	bool bottom = (cutoff & cutoffValues::bottom) != 0;
	bool right = (cutoff & cutoffValues::right) != 0;
	bool left = (cutoff & cutoffValues::left) != 0;
	if (top) {
		min_top = { (35 * ((cutoff & cutoffValues::left_top_corner_left != 0) ? 0 : 1)), height * 70 - 35 };
		max_top = { width * 70 - (35 * ((cutoff & cutoffValues::right_top_corner_right != 0) ? 0 : 1)), height * 70 };
	}
	if (bottom) {
		min_bottom = { (35 * ((cutoff & cutoffValues::left_bottom_corner_left != 0) ? 0 : 1)), 0 };
		max_bottom = { width * 70 - (35 * ((cutoff & cutoffValues::right_bottom_corner_right != 0) ? 0 : 1)), 35 };
	}
	if (right) {
		min_right = { width*70-35,(35 * ((cutoff & cutoffValues::right_bottom_corner_bottom != 0) ? 0 : 1)) };
		max_right = { width * 70, height*70 - (35 * ((cutoff & cutoffValues::right_top_corner_top != 0) ? 0 : 1)) };
	}
	if (left) {
		min_left = { 0, (35 * ((cutoff & cutoffValues::left_top_corner_top != 0) ? 0 : 1)) };
		max_left = { 35, height * 70 - (35 * ((cutoff & cutoffValues::right_top_corner_top != 0) ? 0 : 1)) };
	}


	std::unordered_set<size_t> imdicesToErase;

	for (size_t i = 0 ;  i < (meshOut.m_Vertices.size() / 3); ++i) {
		bool toBeDeleted = true;
		for (uint32_t j = 0; j < 3; j++) {
			if (isInsideRect(meshOut.m_Vertices[3 * i + j].pos, min_top, max_top)) {
				toBeDeleted = false;
				break;
			}
			if (isInsideRect(meshOut.m_Vertices[3 * i + j].pos, min_right, max_right)) {
				toBeDeleted = false;
				break;
			}
			if (isInsideRect(meshOut.m_Vertices[3 * i + j].pos, min_bottom, max_bottom)) {
				toBeDeleted = false;
				break;
			}
			if (isInsideRect(meshOut.m_Vertices[3 * i + j].pos, min_left, max_left)) {
				toBeDeleted = false;
				break;
			}
		}
		if (toBeDeleted) {
			imdicesToErase.insert(3*i);
			imdicesToErase.insert(3*i+1);
			imdicesToErase.insert(3*i+2);
		} 
	}
	std::vector<Vertex> result;
	result.reserve(meshOut.m_Vertices.size() - imdicesToErase.size());

	for (size_t i = 0; i < meshOut.m_Vertices.size(); ++i) {
		if (!imdicesToErase.count(i)) {
			result.push_back(meshOut.m_Vertices[i]);
		}
	}

	meshOut.m_Vertices = std::move(result);
}
