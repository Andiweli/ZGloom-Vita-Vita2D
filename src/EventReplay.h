#pragma once
#include <cstdint>
#include <vector>
class GloomMap;
namespace EventReplay
{
    void Clear();
    void Record(uint32_t ev);
    void SetEvents(const std::vector<uint32_t>& events);
    const std::vector<uint32_t>& GetEvents();
    bool HasReplay();
    bool LoadFromDisk();
    bool SaveToDisk();
    void ReplayAll(GloomMap& map);
}
