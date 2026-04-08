#pragma once
#include <glm/glm.hpp>
#include "renderer_p/image/bindable_image.h"

namespace rfct {
    struct GlyphVertex {
        glm::vec2 pos;
        glm::vec2 texCoord;
        glm::vec3 color;
		int texIndex;

        static vk::VertexInputBindingDescription getBindingDescription() {
            vk::VertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(GlyphVertex);
            bindingDescription.inputRate = vk::VertexInputRate::eVertex;
            return bindingDescription;
        }
        static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
            std::array<vk::VertexInputAttributeDescription, 4> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
            attributeDescriptions[0].offset = offsetof(GlyphVertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = vk::Format::eR32G32Sfloat;
            attributeDescriptions[1].offset = offsetof(GlyphVertex, texCoord);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[2].offset = offsetof(GlyphVertex, color);

            attributeDescriptions[3].binding = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = vk::Format::eR32Sint;
            attributeDescriptions[3].offset = offsetof(GlyphVertex, texIndex);

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
    // font is just a texture atlas to hold the font and info on glyphs
	class font {
	public:
        const glyph* getGlyph(char character) const;
		font(const std::string& path); // path should point to a .txt
        float getTextWidth(const std::string& text, float scale);
    public:
		bindableImage m_TextureAtlas;
		std::unordered_map<char, glyph> glyphMap;
        float fontScale;
	};
}