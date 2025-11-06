#include "vita/RendererHooks.h"
#include "vita/RendererVita2D.h"
#ifdef __vita__
#include <vita2d.h>
#endif
#include <SDL2/SDL.h>
#include <stdint.h>
#include <math.h>
#include <atomic>
#include <stdlib.h>
#include <algorithm> // std::min

namespace Config {
int  GetVignetteEnabled();
int  GetVignetteStrength();
int  GetVignetteRadius();
int  GetVignetteSoftness();
int  GetVignetteWarmth();            // may be 0..5, 0..10, -100..100, or 0/1
int  GetFilmGrain();                 // 0..5
int  GetFilmGrainIntensity();        // 0..5
int  GetScanlines();                 // 0..5
int  GetScanlineIntensity();         // 0..5
}

namespace {
std::atomic<bool>  g_worldFrame{false};
std::atomic<int>   g_screenW{960}, g_screenH{544};
std::atomic<int>   g_targetFps{60};

#ifdef __vita__
vita2d_texture* g_noiseTex = nullptr;
vita2d_texture* g_vignetteTex = nullptr;
#endif

int g_vigLastEnabled=-1, g_vigLastStrength=-1, g_vigLastRadius=-1, g_vigLastSoftness=-1;
uint32_t g_lastTicks = 0;

static inline uint8_t clamp8(int v){ return (uint8_t)(v<0?0:(v>255?255:v)); }

#ifdef __vita__
static void ensure_noise() {
    if (g_noiseTex) return;
    const int N=128;
    g_noiseTex = vita2d_create_empty_texture_format(N, N, SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
    vita2d_texture_set_filters(g_noiseTex, SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);
}

static void refresh_noise_frame() {
    if (!g_noiseTex) return;
    const int Nw = vita2d_texture_get_width(g_noiseTex);
    const int Nh = vita2d_texture_get_height(g_noiseTex);
    uint32_t* d = (uint32_t*)vita2d_texture_get_datap(g_noiseTex);
    const int stride = vita2d_texture_get_stride(g_noiseTex)/4;
    for (int y=0; y<Nh; ++y) {
        for (int x=0; x<Nw; ++x) {
            uint8_t n = (uint8_t)(rand() & 0xFF);
            d[y*stride+x] = 0xFF000000u | (n<<16) | (n<<8) | n;
        }
    }
}

static void rebuild_vignette_if_needed() {
    int en = Config::GetVignetteEnabled();
    int st = Config::GetVignetteStrength();   // 0..5
    int ra = Config::GetVignetteRadius();     // 0..5
    int so = Config::GetVignetteSoftness();   // 0..5

    if (g_vignetteTex && en==g_vigLastEnabled && st==g_vigLastStrength && ra==g_vigLastRadius && so==g_vigLastSoftness)
        return;

    if (g_vignetteTex) { vita2d_free_texture(g_vignetteTex); g_vignetteTex=nullptr; }

    if (!en) { g_vigLastEnabled=en; g_vigLastStrength=st; g_vigLastRadius=ra; g_vigLastSoftness=so; return; }

    const int S=256;
    g_vignetteTex = vita2d_create_empty_texture_format(S, S, SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
    uint32_t* d = (uint32_t*)vita2d_texture_get_datap(g_vignetteTex);
    const int stride = vita2d_texture_get_stride(g_vignetteTex)/4;

    float baseRadius = 0.60f + 0.06f * ra;
    float softness   = 0.08f + 0.04f * so;
    float strengthA  = 0.18f * st;

    for (int y=0;y<S;++y){
        for (int x=0;x<S;++x){
            float nx = (x + 0.5f)/S * 2.f - 1.f;
            float ny = (y + 0.5f)/S * 2.f - 1.f;
            float r  = sqrtf(nx*nx + ny*ny);
            float a  = 0.f;
            if (r > baseRadius){
                a = (r - baseRadius) / softness;
                if (a < 0.f) a = 0.f;
                if (a > 1.f) a = 1.f;
            }
            uint8_t A = clamp8((int)(a * strengthA * 255.f));
            d[y*stride+x] = (A<<24) | (255<<16) | (255<<8) | 255;
        }
    }
    vita2d_texture_set_filters(g_vignetteTex, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);

    g_vigLastEnabled=en; g_vigLastStrength=st; g_vigLastRadius=ra; g_vigLastSoftness=so;
}

// Read a normalized warmth 0..10 from storage (supports 0..5 legacy)
static inline int Warmth01_10() {
    int w = Config::GetVignetteWarmth();
    if (w >= 0 && w <= 5)  return w * 2;
    if (w >= 0 && w <= 10) return w;
    if (w == 0 || w == 1)  return w * 10;
    if (w < -100) w = -100;
    if (w >  100) w =  100;
    return (w + 100) / 20;
}
#endif

} // anon

void EnqueueLensFlareScreen(int, int, float, float) {}

namespace RendererHooks {

bool init(SDL_Renderer*, int screenW, int screenH) {
    g_screenW = screenW; g_screenH = screenH;
    g_worldFrame = false;
    g_lastTicks = SDL_GetTicks();
    return true;
}

void shutdown() {
#ifdef __vita__
    if (g_noiseTex){ vita2d_free_texture(g_noiseTex); g_noiseTex=nullptr; }
    if (g_vignetteTex){ vita2d_free_texture(g_vignetteTex); g_vignetteTex=nullptr; }
#endif
}

void setRenderSize(int, int) {}
void setTargetFps(int fps) { g_targetFps = (fps < 45) ? 30 : 60; }
void setVignetteLevel(int) {}
void setScanlineLevel(int) {}
void setFilmGrainLevel(int) {}

void beginFrame(){ g_worldFrame = false; }
void markWorldFrame(){ g_worldFrame = true; }

void setCameraMotion(float, float, float) {}
void endFramePresent(){}

void EffectsDrawOverlaysVita2D() {
#ifdef __vita__
    if (!zgloom_vita::RendererVita2D::Get().IsInitialized()) return;
    if (!g_worldFrame.load()) return;

    const int W = g_screenW.load();
    const int H = g_screenH.load();

    // Film Grain
    int fg  = Config::GetFilmGrain();
    int fgi = Config::GetFilmGrainIntensity();
    if (fg > 0 && fgi > 0) {
        ensure_noise();
        refresh_noise_frame();
        float alpha = (float)(fg * fgi) * (0.26f / 25.0f);
        if (alpha > 0.f && g_noiseTex) {
            float sx = (float)W / (float)vita2d_texture_get_width(g_noiseTex);
            float sy = (float)H / (float)vita2d_texture_get_height(g_noiseTex);
            uint32_t tint = RGBA8(255,255,255, (int)(alpha * 255.f));
            vita2d_draw_texture_tint_scale(g_noiseTex, 0.f, 0.f, sx, sy, tint);
        }
    }

    // Scanlines
    int sl  = Config::GetScanlines();
    int sli = Config::GetScanlineIntensity();
    if (sl > 0 && sli > 0) {
        int ilvl = std::min(sli, 5);
        float a = (float)ilvl * 0.14f;
        if (a > 0.60f) a = 0.60f;
        uint32_t colA = RGBA8(0,0,0, (int)(a * 255.f));
        for (int y=0; y<H; y+=2) {
            vita2d_draw_rectangle(0.0f, (float)y, (float)W, 1.0f, colA);
        }
    }

    // Vignette (neutral)
    rebuild_vignette_if_needed();
    if (g_vignetteTex) {
        float sx = (float)W / 256.0f;
        float sy = (float)H / 256.0f;
        uint32_t vigTint = RGBA8(0,0,0, 255);
        vita2d_draw_texture_tint_scale(g_vignetteTex, 0.f, 0.f, sx, sy, vigTint);
    }

    // Warm grade: apply only when Vignette ON and slider >=6
    if (Config::GetVignetteEnabled()) {
        int w01_10 = Warmth01_10();        // 0..10
        if (w01_10 >= 6) {
            float scale = (w01_10 - 5) / 5.0f; // 6..10 -> 0.2..1.0 (linear)
            float baseA = 0.14f;
            float a = baseA * scale;
            uint32_t warmTint = RGBA8(255, 190, 110, (int)(a * 255.f));
            vita2d_draw_rectangle(0.f, 0.f, (float)W, (float)H, warmTint);
        }
    }

    // FPS limiter (only at 30)
    int tfps = g_targetFps.load();
    if (tfps == 30) {
        uint32_t targetMs = 33;
        uint32_t now = SDL_GetTicks();
        uint32_t dt  = now - g_lastTicks;
        if (dt < targetMs) SDL_Delay(targetMs - dt);
        g_lastTicks = SDL_GetTicks();
    }
#endif
}

int GetParticleDustEnabled(){ return 0; }
void SetParticleDustEnabled(int) {}

} // namespace RendererHooks
