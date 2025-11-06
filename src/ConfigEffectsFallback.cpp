// src/ConfigEffectsFallback.cpp — stub implementations for effects config
// Provides missing symbols referenced by menus & save code when the full effects system is not compiled.
#include <algorithm>

namespace Config {

static int gVignetteEnabled     = 0;
static int gVignetteStrength    = 0; // 0..5
static int gVignetteRadius      = 0; // 0..5
static int gVignetteSoftness    = 0; // 0..5
static int gVignetteWarmth      = 0; // 0..5

static int gFilmGrain           = 0; // 0..5
static int gFilmGrainIntensity  = 0; // 0..5

static int gScanlines           = 0; // 0..5
static int gScanlineIntensity   = 0; // 0..5

inline int clamp05(int v){ return std::max(0, std::min(5, v)); }

void EffectsConfigInit() {
    // No-op in fallback; defaults remain.
}

// Vignette
void SetVignetteEnabled(int v)   { gVignetteEnabled  = v ? 1 : 0; }
int  GetVignetteEnabled()        { return gVignetteEnabled; }

void SetVignetteStrength(int v)  { gVignetteStrength = clamp05(v); }
int  GetVignetteStrength()       { return gVignetteStrength; }

void SetVignetteRadius(int v)    { gVignetteRadius   = clamp05(v); }
int  GetVignetteRadius()         { return gVignetteRadius; }

void SetVignetteSoftness(int v)  { gVignetteSoftness = clamp05(v); }
int  GetVignetteSoftness()       { return gVignetteSoftness; }

void SetVignetteWarmth(int v)    { gVignetteWarmth   = clamp05(v); }
int  GetVignetteWarmth()         { return gVignetteWarmth; }

// Film grain
void SetFilmGrain(int v)         { gFilmGrain = clamp05(v); }
int  GetFilmGrain()              { return gFilmGrain; }

void SetFilmGrainIntensity(int v){ gFilmGrainIntensity = clamp05(v); }
int  GetFilmGrainIntensity()     { return gFilmGrainIntensity; }

// Scanlines
void SetScanlines(int v)         { gScanlines = clamp05(v); }
int  GetScanlines()              { return gScanlines; }

void SetScanlineIntensity(int v) { gScanlineIntensity = clamp05(v); }
int  GetScanlineIntensity()      { return gScanlineIntensity; }

} // namespace Config

// Menu adapters used by menuscreen to map warmth to boolean UI toggles in some builds.
namespace MenuAdapters {
int GetWarmth01() {
    return Config::GetVignetteWarmth() > 0 ? 1 : 0;
}
void SetWarmth01(int on) {
    Config::SetVignetteWarmth( on ? 1 : 0 );
}
} // namespace MenuAdapters
