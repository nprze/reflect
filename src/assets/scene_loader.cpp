#include "scene_loader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <charconv>

void rfct::sceneLoader::loadScene(const std::string& path, sceneSerializedData* out)
{
    // IO only here
    std::ifstream file(path);
    if (!file.is_open()) {
        RFCT_CRITICAL("Failed to open scene file: {}", path);
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string_view sv(line);


        if (sv.starts_with("Rect:")) {
            rectangle r{};
            std::getline(file, line); // color
            r.color = line.substr(line.find(':') + 2);

            std::getline(file, line); // min
            {
                int x, y;
                std::from_chars(line.data() + 8, line.data() + line.find(','), x);
                std::from_chars(line.data() + line.find(',') + 2, line.data() + line.size() - 1, y);
                r.min = { x, y };
            }

            std::getline(file, line); // max
            {
                int x, y;
                std::from_chars(line.data() + 8, line.data() + line.find(','), x);
                std::from_chars(line.data() + line.find(',') + 2, line.data() + line.size() - 1, y);
                r.max = { x, y };
            }

            std::getline(file, line); // file
            r.file = line.substr(line.find(':') + 2);
            if (!r.file.empty() && r.file.back() == '\r') {
                r.file.pop_back();
            }

            out->rectangles.push_back(std::move(r));
        }
        else if (sv.starts_with("Vine:")) {
            vineInfo vn = {};
            bool vineFullyOk = false;
            float xin, yin;
            while (!vineFullyOk) {
                std::getline(file, line);
                if (line.find("start:") != std::string::npos) {

                    RFCT_ASSERT(sscanf(line.c_str(), "  start: (%f, %f)", &xin, &yin) == 2);
                    vn.start.x = xin;
                    vn.start.y = yin;
                }
                else if (line.find("end:") != std::string::npos) {
                    RFCT_ASSERT(sscanf(line.c_str(), "  end: (%f, %f)", &xin, &yin) == 2);
                    vn.end.x = xin;
                    vn.end.y = yin;
                }
                else if (line.find("edges:") != std::string::npos) {
                    RFCT_ASSERT(sscanf(line.c_str(), "  edges: %d", &vn.numEdges) == 1);
                    vineFullyOk = true;
                }
            }
            out->vines.push_back(vn);
        }
        else if (sv.starts_with("NPC:")) {
            NPCInfo npc{};

            std::getline(file, line); // min
            {
                float x = std::stof(line.substr(10, line.find(',') - 10));
                float y = std::stof(line.substr(line.find(',') + 2, line.size() - line.find(',') - 3));
                npc.min = { x, y };
            }

            std::getline(file, line); // max
            {
                float x = std::stof(line.substr(8, line.find(',') - 8));
                float y = std::stof(line.substr(line.find(',') + 2, line.size() - line.find(',') - 3));
                npc.max = { x, y };
            }

            std::getline(file, line); // interaction radius
            float count;
            RFCT_ASSERT(sscanf(line.c_str(), "  interactionRadius: %f", &count) == 1);
            npc.ineratcionRadius = count;


            std::getline(file, line); // file
            npc.dialogueFile = line.substr(line.find(':') + 2);
            if (!npc.dialogueFile.empty() && npc.dialogueFile.back() == '\r') {
                npc.dialogueFile.pop_back();
            }

            out->npcs.push_back(std::move(npc));
        }
        else if (sv.starts_with("Spike:")) {
            SpikeInfo spk{};

            std::getline(file, line); // min
            {
                float x = std::stof(line.substr(10, line.find(',') - 10));
                float y = std::stof(line.substr(line.find(',') + 2, line.size() - line.find(',') - 3));
                spk.min = { x, y };
            }

            std::getline(file, line); // max
            {
                float x = std::stof(line.substr(8, line.find(',') - 8));
                float y = std::stof(line.substr(line.find(',') + 2, line.size() - line.find(',') - 3));
                spk.max = { x, y };
            }

            std::getline(file, line); // dir
            std::string dirString = line.substr(line.find(':') + 2);
            if (!dirString.empty() && dirString.back() == '\r') {
                dirString.pop_back();
            }
            if (dirString == "up") {
                spk.dir = SpikeDirection::up;
            }else if (dirString == "down") {
                spk.dir = SpikeDirection::down;
            }else if (dirString == "right") {
                spk.dir = SpikeDirection::right;
            }else if (dirString == "left") {
                spk.dir = SpikeDirection::left;
            }
            out->spikes.push_back(std::move(spk));
        }
        else if (sv.starts_with("BasicEnemy:")) {
            BasicEnemyInfo enemy{};

            std::getline(file, line); // position
            {
                float x = std::stof(line.substr(13, line.find(',') - 10));
                float y = std::stof(line.substr(line.find(',') + 2, line.size() - line.find(',') - 3));
                enemy.position = { x, y };
            }

            std::getline(file, line); // min
            {
                float x = std::stof(line.substr(8, line.find(',') - 8));
                float y = std::stof(line.substr(line.find(',') + 2, line.size() - line.find(',') - 3));
                enemy.min = { x, y };
            }

            std::getline(file, line); // max
            {
                float x = std::stof(line.substr(8, line.find(',') - 8));
                float y = std::stof(line.substr(line.find(',') + 2, line.size() - line.find(',') - 3));
                enemy.max = { x, y };
            }

            std::getline(file, line); // file
            enemy.animation = line.substr(line.find(':') + 2);
            if (!enemy.animation.empty() && enemy.animation.back() == '\r') {
                enemy.animation.pop_back();
            }
            out->enemies.push_back(std::move(enemy));
        }
        else if (sv.starts_with("SceneWidth:")) {
            std::from_chars(sv.data() + 12, sv.data() + sv.size(), out->width);
        }
        else if (sv.starts_with("SceneHeight:")) {
            std::from_chars(sv.data() + 13, sv.data() + sv.size(), out->height);
        }
        else if (sv.starts_with("RectCount:")) {
            int count;
            std::from_chars(sv.data() + 11, sv.data() + sv.size(), count);
            out->rectangles.reserve(count);
        }
        else if (sv.starts_with("VineCount:")) {
            int count;
            std::from_chars(sv.data() + 11, sv.data() + sv.size(), count);
            out->vines.reserve(count);
        }
        else if (sv.starts_with("SpikeCount:")) {
            int count;
            std::from_chars(sv.data() + 12, sv.data() + sv.size(), count);
            out->spikes.reserve(count);
        }
        else if (sv.starts_with("BasicEnemyCount:")) {
            int count;
            std::from_chars(sv.data() + 17, sv.data() + sv.size(), count);
            out->enemies.reserve(count);
        }
        else if (sv.starts_with("NPCCount:")) {
            int count;
            std::from_chars(sv.data() + 10, sv.data() + sv.size(), count);
            out->npcs.reserve(count);
        }
        else if (sv.starts_with("SpawnPoint:")) {
            float xin, yin;
            RFCT_ASSERT(sscanf(line.c_str(), "SpawnPoint: (%f, %f)", &xin, &yin) == 2);
            out->spawnPoint.x = xin;
            out->spawnPoint.y = yin;
        }
    }
}
