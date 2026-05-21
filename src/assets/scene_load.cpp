#include "object_load.h"
#include <iostream>
#include <fstream>
#include "assets_utils.h"
#include "serialize_structures/scene_serialize_data.h"

#define FILE_COUNT_PART(label, out)                                                                                 \
    else if (sv.starts_with(label)) {                                                                               \
            int count;                                                                                              \
            std::from_chars(sv.data() + sizeof(label), sv.data() + sv.size(), count);                               \
            out.reserve(count);                                                                                     \
        }              

#define FILE_STRING(label, out)                                                                                     \
    { std::getline(file, line);                                                                                     \
    out = line.substr(sizeof(label) + 2);                                                                           \
    while (!out.empty() && out.back() == '\r') out.pop_back(); }

#define FILE_VEC2_INT(label, out)                                                                                   \
std::getline(file, line);                                                                                           \
{                                                                                                                   \
    int x, y;                                                                                                       \
    std::from_chars(line.data() + line.find(label) + sizeof(label) + 1, line.data() + line.find(','), x);           \
    std::from_chars(line.data() + line.find(',') + 2, line.data() + line.size() - 1, y);                            \
    out = { x, y };                                                                                                 \
}

#define FILE_VEC2_FLOAT(label, out)                                                                                 \
std::getline(file, line);                                                                                           \
{                                                                                                                   \
    float xin, yin;                                                                                                 \
    RFCT_ASSERT(sscanf(line.c_str(), "  " label " (%f, %f)", &xin, &yin) == 2);                                     \
    out.x = xin;                                                                                                    \
    out.y = yin;                                                                                                    \
}                                                                                                                   

#define FILE_INT(label, out) std::getline(file, line); RFCT_ASSERT(sscanf(line.c_str(), "  " label " %d", &out) == 1);

#define FILE_FLOAT(label, out) std::getline(file, line); RFCT_ASSERT(sscanf(line.c_str(), "  " label " %f", &out) == 1);                                                                                  



void rfct::loadScene(const std::string& path, sceneSerializedData* out) {
    RFCT_PROFILE_FUNCTION();
    std::ifstream file;
    if (!openAssetFile(path, &file)) {
        RFCT_CRITICAL("Could not open scene file:  {}", path);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string_view sv(line);

        if (sv.starts_with("Rect:")) {
            rectangle r{};

            FILE_STRING("color:", r.color);
            FILE_VEC2_INT("min:", r.min);
            FILE_VEC2_INT("max:", r.max);
            FILE_STRING("file:", r.file);

            out->rectangles.push_back(std::move(r));
        }
        else if (sv.starts_with("Vine:")) {
            vineInfo vn = {};

            FILE_VEC2_FLOAT("start:", vn.start);
            FILE_VEC2_FLOAT("end:", vn.end);
            FILE_INT("edges:", vn.numEdges);

            out->vines.push_back(vn);
        }
        else if (sv.starts_with("NPC:")) {
            NPCInfo npc{};

            FILE_VEC2_FLOAT("start:", npc.min);
            FILE_VEC2_FLOAT("end:", npc.max);
            FILE_FLOAT("interactionRadius:", npc.interatcionRadius);
            FILE_STRING("dialogueName:", npc.dialogueFile);

            out->npcs.push_back(std::move(npc));
        }
        else if (sv.starts_with("Spike:")) {
            SpikeInfo spk{};
            std::string dirString;

            FILE_VEC2_FLOAT("start:", spk.min);
            FILE_VEC2_FLOAT("end:", spk.max);
            FILE_STRING("dir:", dirString);

            if (dirString == "up") {
                spk.dir = SpikeDirection::SpikeDirUp;
            }
            else if (dirString == "down") {
                spk.dir = SpikeDirection::SpikeDirDown;
            }
            else if (dirString == "right") {
                spk.dir = SpikeDirection::SpikeDirRight;
            }
            else if (dirString == "left") {
                spk.dir = SpikeDirection::SpikeDirLeft;
            }

            out->spikes.push_back(std::move(spk));
        }
        else if (sv.starts_with("Enemy:")) {
            BasicEnemyInfo enemy{};

            FILE_VEC2_FLOAT("position:", enemy.position);
            FILE_VEC2_FLOAT("min:", enemy.min);
            FILE_VEC2_FLOAT("max:", enemy.max);
            FILE_STRING("animName:", enemy.animation);

            out->enemies.push_back(std::move(enemy));
        }
        else if (sv.starts_with("JumpBooster:")) {
            JumpBoosterInfo booster{};

            FILE_VEC2_FLOAT("position:", booster.position);

            out->boosters.push_back(std::move(booster));
        }
        else if (sv.starts_with("DashRecharge:")) {
            DashRechargeInfo recharge{};

            FILE_VEC2_FLOAT("position:", recharge.position);

            out->dashRecharge.push_back(std::move(recharge));
        }
        else if (sv.starts_with("SpawnPoint:")) {
            SpawnPointInfo spawn{};

            FILE_VEC2_FLOAT("position:", spawn.position);

            out->spawnPoints.push_back(std::move(spawn));
        }
        else if (sv.starts_with("TallGrass:")) {
            TallGrassInfo grass{};

            FILE_VEC2_FLOAT("position:", grass.position);

            grass.position.x += 0;
            grass.position.y += 0.5;

            out->tallGrass.push_back(std::move(grass));
        }
        else if (sv.starts_with("SceneWidth:")) {
            std::from_chars(sv.data() + 12, sv.data() + sv.size(), out->width);
        }
        else if (sv.starts_with("SceneHeight:")) {
            std::from_chars(sv.data() + 13, sv.data() + sv.size(), out->height);
        }
        FILE_COUNT_PART("RectCount:", out->rectangles)
        FILE_COUNT_PART("VineCount:", out->vines)
        FILE_COUNT_PART("SpikeCount:", out->spikes)
        FILE_COUNT_PART("EnemyCount:", out->enemies)
        FILE_COUNT_PART("NPCCount:", out->npcs)
        FILE_COUNT_PART("SpawnPointCount:", out->spawnPoints)
        FILE_COUNT_PART("DashRechargeCount:", out->dashRecharge)
        FILE_COUNT_PART("DashRechargeCount:", out->dashRecharge)
        FILE_COUNT_PART("JumpBoosterCount:", out->boosters)
        FILE_COUNT_PART("TallGrassCount:", out->tallGrass)
    }
}