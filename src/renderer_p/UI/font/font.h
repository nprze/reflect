#pragma once
#include "renderer_p\image\bindable_image.h"
#include <glm\glm.hpp>
namespace rfct {
	struct GlyphVertex {
		glm::vec2 pos;
		glm::vec2 texCoord;

		static vk::VertexInputBindingDescription getBindingDescription() {
			vk::VertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(GlyphVertex);
			bindingDescription.inputRate = vk::VertexInputRate::eVertex;
			return bindingDescription;
		}

		static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
			std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
			attributeDescriptions[0].offset = offsetof(GlyphVertex, pos);
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
			attributeDescriptions[1].offset = offsetof(GlyphVertex, texCoord);
			return attributeDescriptions;
		}
	};
	struct glyph {
		float x;
		float y;
		float width;
		float height;
		float xoffset;
		float yoffset;
		float xadvance;
	};
	class font {
	public:
		inline const glyph* getGlyph(char character) const {
			auto it = glyphMap.find(character);
			if (it == glyphMap.end()) RFCT_CRITICAL("trying to get a glyph which isn't in the font");
			return &it->second;
		}
		font(std::string path);
		bindableImage m_TextureAtlas;
		std::unordered_map<char, glyph> glyphMap;
	};
}