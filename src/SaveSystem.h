#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct Camera;
class GloomMap;

namespace SaveSystem
{
    struct SaveData
    {
        int formatVersion = 2;
        std::string levelPath;
        int flatIndex = -1;
        int camX = 0;
        int camY = 0;
        int camZ = 0;
        int camRot = 0;
        int hp = 100;
        int lives = 3;
        int weapon = 0;
        int reload = 0;
        int reloadcnt = 0;
        std::vector<uint32_t> eventHistory;
    };

    bool HasSave();
    bool HasSaveForCurrentGame();
    bool LoadFromDisk(SaveData& outData);
    bool SaveToDisk(const SaveData& inData);

    void SetCurrentLevelPath(const std::string& levelPath);
    const std::string& GetCurrentLevelPath();
    void SetCurrentFlat(int flatIndex);
    int GetCurrentFlat();

    // Vita compatibility helper retained for old call sites.
    bool ApplyToGame(const SaveData& data, Camera& cam, GloomMap& gmap);
}
