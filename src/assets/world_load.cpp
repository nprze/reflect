#include "object_load.h"
#include <fstream>
#include "assets_utils.h"
#include "serialize_structures/world_serialize_data.h"

void rfct::loadWorld(const std::string& path, worldSerializeData* out) {
    RFCT_PROFILE_FUNCTION();
    std::ifstream file;
    if (!OpenAssetFile(path, &file)) {
        RFCT_CRITICAL("Could not open world file: {}", path);
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty())
            continue;

        std::stringstream ss(line);

        std::string namePart;
        float x1, y1, x2, y2;

        if (!(ss >> namePart >> x1 >> y1 >> x2 >> y2)) {
            RFCT_CRITICAL("Invalid world line format: {}", line);
            continue;
        }

        blockSerializeData block;
        block.file = namePart;
        block.min = glm::vec2(x1, y1);
        block.max = glm::vec2(x2, y2);

        out->blocks.push_back(block);
    }
}
