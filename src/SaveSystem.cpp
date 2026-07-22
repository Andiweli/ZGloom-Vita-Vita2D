#include "SaveSystem.h"

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <sys/stat.h>

#include "config.h"
#include "gloommap.h"
#include "objectgraphics.h"
#include "renderer.h"

namespace
{
    std::string g_currentLevelPath;
    int g_currentFlat = -1;

    const char* kMagicV1 = "ZGLOOM_SAVE_V1";
    const char* kMagicV2 = "ZGLOOM_SAVE_V2";

    std::string GameKey()
    {
        switch (Config::GetGameID())
        {
            case Config::GLOOM: return "gloom";
            case Config::DELUXE: return "deluxe";
            case Config::GLOOM3: return "gloom3";
            case Config::MASSACRE: return "massacre";
            default: return "custom";
        }
    }

    void EnsureDir(const std::string& path)
    {
        ::mkdir(path.c_str(), 0777);
    }

    std::string BuildSaveDir()
    {
        const std::string root = "ux0:/data/ZGloom";
        const std::string saves = root + "/saves";
        const std::string dir = saves + "/" + GameKey();
        EnsureDir(root);
        EnsureDir(saves);
        EnsureDir(dir);
        return dir;
    }

    std::string BuildSavePath() { return BuildSaveDir() + "/savepos.txt"; }
    std::string BuildLegacyPath() { return BuildSaveDir() + "/last.sav"; }

    void StripLineEnd(char* line)
    {
        size_t len = std::strlen(line);
        while (len && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';
    }

    bool ParseInt(const char* text, int& value)
    {
        if (!text || !*text) return false;
        errno = 0;
        char* end = nullptr;
        const long parsed = std::strtol(text, &end, 10);
        if (errno || end == text || !end || *end != '\0' || parsed < INT_MIN || parsed > INT_MAX)
            return false;
        value = static_cast<int>(parsed);
        return true;
    }

    bool ParseUInt32(const char* text, uint32_t& value)
    {
        if (!text || !*text || *text == '-') return false;
        errno = 0;
        char* end = nullptr;
        const unsigned long parsed = std::strtoul(text, &end, 10);
        if (errno || end == text || !end || *end != '\0' || parsed > 0xFFFFFFFFUL)
            return false;
        value = static_cast<uint32_t>(parsed);
        return true;
    }

    bool CommitTemporarySave(const std::string& temp, const std::string& finalPath)
    {
        if (std::rename(temp.c_str(), finalPath.c_str()) == 0) return true;
        const std::string backup = finalPath + ".bak";
        std::remove(backup.c_str());
        const bool hadOld = std::rename(finalPath.c_str(), backup.c_str()) == 0;
        if (std::rename(temp.c_str(), finalPath.c_str()) == 0)
        {
            if (hadOld) std::remove(backup.c_str());
            return true;
        }
        if (hadOld) std::rename(backup.c_str(), finalPath.c_str());
        std::remove(temp.c_str());
        return false;
    }

    bool LoadLegacy(SaveSystem::SaveData& out)
    {
        std::ifstream in(BuildLegacyPath().c_str(), std::ios::binary);
        if (!in) return false;
        SaveSystem::SaveData tmp;
        tmp.formatVersion = 1;
        std::string line;
        while (std::getline(in, line))
        {
            const std::string::size_type pos = line.find('=');
            if (pos == std::string::npos) continue;
            const std::string key = line.substr(0, pos);
            const std::string value = line.substr(pos + 1);
            const int number = std::atoi(value.c_str());
            if (key == "level") tmp.levelPath = value;
            else if (key == "x") tmp.camX = number;
            else if (key == "z") tmp.camZ = number;
            else if (key == "rot") tmp.camRot = number & 0xFF;
            else if (key == "hp") tmp.hp = number;
            else if (key == "weapon") tmp.weapon = number;
            else if (key == "reload") tmp.reload = number;
            else if (key == "reloadcnt") tmp.reloadcnt = number;
            else if (key == "flat") tmp.flatIndex = number;
        }
        if (tmp.levelPath.empty()) return false;
        out = tmp;
        return true;
    }
}

namespace SaveSystem
{
    bool HasSave()
    {
        FILE* f = std::fopen(BuildSavePath().c_str(), "rb");
        if (f)
        {
            char magic[64] = {0};
            const bool ok = std::fgets(magic, sizeof(magic), f) != nullptr;
            std::fclose(f);
            if (ok)
            {
                StripLineEnd(magic);
                if (std::strcmp(magic, kMagicV1) == 0 || std::strcmp(magic, kMagicV2) == 0)
                    return true;
            }
        }
        f = std::fopen(BuildLegacyPath().c_str(), "rb");
        if (!f) return false;
        std::fclose(f);
        return true;
    }

    bool HasSaveForCurrentGame() { return HasSave(); }

    bool LoadFromDisk(SaveData& outData)
    {
        FILE* f = std::fopen(BuildSavePath().c_str(), "rb");
        if (!f) return LoadLegacy(outData);

        char line[512];
        if (!std::fgets(line, sizeof(line), f)) { std::fclose(f); return false; }
        StripLineEnd(line);
        SaveData tmp;
        if (std::strcmp(line, kMagicV2) == 0) tmp.formatVersion = 2;
        else if (std::strcmp(line, kMagicV1) == 0) tmp.formatVersion = 1;
        else { std::fclose(f); return false; }

        int expectedEventCount = -1;
        bool parseError = false;
        while (std::fgets(line, sizeof(line), f))
        {
            StripLineEnd(line);
            if (std::strncmp(line, "LEVEL=", 6) == 0) tmp.levelPath = line + 6;
            else if (std::strncmp(line, "FLAT=", 5) == 0) parseError |= !ParseInt(line + 5, tmp.flatIndex);
            else if (std::strncmp(line, "CAMX=", 5) == 0) parseError |= !ParseInt(line + 5, tmp.camX);
            else if (std::strncmp(line, "CAMY=", 5) == 0) parseError |= !ParseInt(line + 5, tmp.camY);
            else if (std::strncmp(line, "CAMZ=", 5) == 0) parseError |= !ParseInt(line + 5, tmp.camZ);
            else if (std::strncmp(line, "CAMROT=", 7) == 0) parseError |= !ParseInt(line + 7, tmp.camRot);
            else if (std::strncmp(line, "HP=", 3) == 0) parseError |= !ParseInt(line + 3, tmp.hp);
            else if (std::strncmp(line, "LIVES=", 6) == 0)
            {
                parseError |= !ParseInt(line + 6, tmp.lives);
                if (tmp.lives < 0 || tmp.lives > 5) parseError = true;
            }
            else if (std::strncmp(line, "WEAPON=", 7) == 0) parseError |= !ParseInt(line + 7, tmp.weapon);
            else if (std::strncmp(line, "RELOAD=", 7) == 0) parseError |= !ParseInt(line + 7, tmp.reload);
            else if (std::strncmp(line, "RELOADCNT=", 10) == 0) parseError |= !ParseInt(line + 10, tmp.reloadcnt);
            else if (tmp.formatVersion >= 2 && std::strncmp(line, "EVENTCOUNT=", 11) == 0)
            {
                parseError |= !ParseInt(line + 11, expectedEventCount);
                if (expectedEventCount < 0 || expectedEventCount > 1024) parseError = true;
            }
            else if (tmp.formatVersion >= 2 && std::strncmp(line, "EVENT=", 6) == 0)
            {
                uint32_t ev = 0;
                if (ParseUInt32(line + 6, ev)) tmp.eventHistory.push_back(ev); else parseError = true;
            }
        }
        const bool ioError = std::ferror(f) != 0;
        std::fclose(f);
        if (ioError || parseError || tmp.levelPath.empty()) return false;
        if (tmp.formatVersion >= 2 && (expectedEventCount < 0 || static_cast<size_t>(expectedEventCount) != tmp.eventHistory.size()))
            return false;
        outData = tmp;
        if (tmp.flatIndex >= 0) g_currentFlat = tmp.flatIndex;
        return true;
    }

    bool SaveToDisk(const SaveData& data)
    {
        if (data.levelPath.empty()) return false;
        const std::string path = BuildSavePath();
        const std::string temp = path + ".tmp";
        FILE* f = std::fopen(temp.c_str(), "wb");
        if (!f) return false;
        bool ok = true;
        ok &= std::fprintf(f, "%s\n", kMagicV2) >= 0;
        ok &= std::fprintf(f, "LEVEL=%s\n", data.levelPath.c_str()) >= 0;
        ok &= std::fprintf(f, "FLAT=%d\n", data.flatIndex) >= 0;
        ok &= std::fprintf(f, "CAMX=%d\nCAMY=%d\nCAMZ=%d\nCAMROT=%d\n", data.camX, data.camY, data.camZ, data.camRot) >= 0;
        ok &= std::fprintf(f, "HP=%d\nLIVES=%d\nWEAPON=%d\nRELOAD=%d\nRELOADCNT=%d\n",
                           data.hp, std::max(0, std::min(5, data.lives)), data.weapon, data.reload, data.reloadcnt) >= 0;
        ok &= std::fprintf(f, "EVENTCOUNT=%lu\n", static_cast<unsigned long>(data.eventHistory.size())) >= 0;
        for (uint32_t ev : data.eventHistory)
            ok &= std::fprintf(f, "EVENT=%lu\n", static_cast<unsigned long>(ev)) >= 0;
        ok &= std::fflush(f) == 0;
        ok &= std::ferror(f) == 0;
        ok &= std::fclose(f) == 0;
        if (!ok) { std::remove(temp.c_str()); return false; }
        return CommitTemporarySave(temp, path);
    }

    void SetCurrentLevelPath(const std::string& levelPath) { g_currentLevelPath = levelPath; }
    const std::string& GetCurrentLevelPath() { return g_currentLevelPath; }
    void SetCurrentFlat(int flatIndex) { g_currentFlat = flatIndex; }
    int GetCurrentFlat() { return g_currentFlat; }

    bool ApplyToGame(const SaveData& d, Camera& cam, GloomMap& gmap)
    {
        if (d.flatIndex >= 0) gmap.SetFlat(static_cast<char>(d.flatIndex));
        cam.x.SetInt(d.camX);
        cam.y = static_cast<int16_t>(d.camY);
        cam.z.SetInt(d.camZ);
        cam.rotquick.SetInt(d.camRot & 0xFF);
        for (MapObject& o : gmap.GetMapObjects())
        {
            if (o.t != ObjectGraphics::OLT_PLAYER1) continue;
            o.x.SetInt(d.camX); o.y.SetInt(d.camY); o.z.SetInt(d.camZ);
            o.data.ms.rotquick.SetInt(d.camRot & 0xFF);
            o.data.ms.hitpoints = std::max(1, std::min(32767, d.hp));
            o.data.ms.weapon = std::max(0, std::min(4, d.weapon));
            o.data.ms.reload = std::max(0, std::min(5, d.reload));
            o.data.ms.reloadcnt = std::max(0, d.reloadcnt);
            break;
        }
        return true;
    }
}
