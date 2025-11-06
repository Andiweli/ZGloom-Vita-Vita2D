#pragma once
#include <SDL2/SDL.h>

void EnqueueLensFlareScreen(int x, int y, float intensity, float scale);

namespace RendererHooks {

bool init(SDL_Renderer* renderer, int screenW, int screenH);
void shutdown();

void setRenderSize(int w, int h);
void setTargetFps(int fps);

void setVignetteLevel(int lvl);
void setScanlineLevel(int lvl);
void setFilmGrainLevel(int lvl);

void beginFrame();
void markWorldFrame();
void endFramePresent();

int  GetParticleDustEnabled();
void SetParticleDustEnabled(int onOff);
void setCameraMotion(float dx, float dy, float dz);

// New: vita2d overlay hook (called while vita2d frame is open)
void EffectsDrawOverlaysVita2D();

// Legacy keepalive (ignored)
inline void EffectsDrawOverlays(SDL_Renderer*) {}

} // namespace RendererHooks
