// Gloom Collection by Andreas 'Andiweli' Stuermer. In honor and memory of Mark Sibly.
// ZGloom PS Vita 08.2026

static const char kZGloomCollectionCredit[] =
    "Gloom Collection by Andreas 'Andiweli' Stuermer. In honor and memory of Mark Sibly.";

#include <psp2/sysmodule.h>
#include <psp2/kernel/clib.h>
#include <psp2/apputil.h>
#include <psp2/ctrl.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/dirent.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <xmp.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "config.h"
#include "vita/RendererHooks.h"
#include "vita/RendererVita2D.h"
#include "gloommap.h"
#include "script.h"
#include "crmfile.h"
#include "iffhandler.h"
#include "renderer.h"
#include "objectgraphics.h"
#include "gamelogic.h"
#include "soundhandler.h"
#include "font.h"
#include "titlescreen.h"
#include "SaveSystem.h"
#include "EventReplay.h"
#include "menuscreen.h"
#include "hud.h"
#include "input.h"
#include "effects/MuzzleFlashFX.h"
#include "BootGameSelect.h"
#include "audio/EmbeddedBGM.h"
#include "audio/AtmosphereVolume.h"
#include "EmbeddedBGMVolume.h"
#include "../assets/gloom_classic_title_fallback_embed.h"
#include "../assets/zombie_massacre_title_fallback_embed.h"

static const unsigned char kEmbeddedBigfont2Crm2[] = {
    0x43, 0x72, 0x4D, 0x32, 0x00, 0x00, 0x00, 0x00, 0x0D, 0xD2, 0x00, 0x00,
    0x04, 0x42, 0x01, 0x76, 0x67, 0x9A, 0x65, 0xC0, 0x16, 0x73, 0x4D, 0x7C,
    0xD1, 0xAE, 0x01, 0xFC, 0x4C, 0xD3, 0x2E, 0x68, 0x37, 0x00, 0x6B, 0x1E,
    0x68, 0x56, 0x68, 0xB7, 0x02, 0x0E, 0x4F, 0x34, 0xEB, 0x9A, 0x58, 0x0D,
    0xFA, 0x9E, 0x68, 0xA7, 0x34, 0xFB, 0x80, 0xBF, 0xB3, 0xCD, 0x26, 0xE6,
    0x89, 0x40, 0xEA, 0x17, 0x9A, 0x2D, 0xCD, 0x05, 0xE0, 0x03, 0xC5, 0xE6,
    0x9D, 0x66, 0x92, 0xF9, 0xA2, 0x5C, 0x0F, 0xBD, 0x33, 0x45, 0x7C, 0xD2,
    0xAE, 0x03, 0xDF, 0x9C, 0xD3, 0x8E, 0x69, 0xD7, 0x02, 0x7E, 0x17, 0x34,
    0x93, 0x9A, 0x4D, 0xC0, 0x1F, 0x89, 0xE6, 0x8D, 0x73, 0x4D, 0xB8, 0x00,
    0x71, 0x73, 0x44, 0xF8, 0x17, 0x71, 0x7A, 0xFB, 0xBF, 0x8E, 0x39, 0x71,
    0xB7, 0x35, 0x05, 0x44, 0x55, 0xC6, 0xDC, 0x8A, 0x52, 0xFF, 0xFE, 0x59,
    0x1A, 0x45, 0x95, 0xC6, 0xFC, 0xD6, 0x21, 0xA2, 0x4B, 0xF7, 0xAE, 0x31,
    0x14, 0xAF, 0xC2, 0x33, 0xBD, 0x1B, 0x1B, 0xCE, 0xB8, 0x46, 0x67, 0x12,
    0x5B, 0x37, 0xC2, 0x5E, 0xB5, 0x4C, 0x19, 0xA5, 0x3A, 0xF2, 0x22, 0x8C,
    0x86, 0x91, 0x1D, 0xB4, 0xC1, 0x1A, 0x3B, 0x34, 0x70, 0xBD, 0xE2, 0xF4,
    0x09, 0xB5, 0xE8, 0x7B, 0xE7, 0x09, 0xD8, 0x1B, 0x15, 0x7B, 0x6F, 0x8B,
    0x30, 0x52, 0xBA, 0xB3, 0xA6, 0x98, 0x13, 0xB7, 0x43, 0xB6, 0x3A, 0xB8,
    0x44, 0xDC, 0x38, 0xB3, 0x6E, 0x7A, 0x27, 0x17, 0x08, 0xC9, 0x3B, 0xA2,
    0x3E, 0xD0, 0xA5, 0x02, 0x74, 0x66, 0xD4, 0x7D, 0x6A, 0x1C, 0x73, 0x26,
    0x71, 0x75, 0xE3, 0x49, 0xE2, 0xC2, 0xEC, 0xC2, 0x9D, 0x7C, 0xDE, 0xA0,
    0x71, 0x0F, 0x7F, 0xA8, 0x73, 0x4C, 0xF0, 0x6F, 0xE7, 0x0F, 0x50, 0x06,
    0x1A, 0xEF, 0xE2, 0xCC, 0x14, 0xAD, 0x08, 0xD1, 0x5E, 0xFE, 0x8A, 0x1A,
    0xD1, 0x3E, 0x6F, 0x7A, 0xDF, 0x22, 0x4C, 0x59, 0x24, 0x4A, 0x91, 0x58,
    0xFE, 0x63, 0x98, 0x48, 0x94, 0x38, 0x01, 0xF9, 0x91, 0x24, 0x00, 0x7E,
    0xE1, 0x4A, 0xFC, 0xD0, 0x01, 0xFA, 0x1A, 0x3A, 0x43, 0xD0, 0x88, 0xF7,
    0x64, 0x3D, 0x0E, 0x14, 0xA6, 0xC6, 0xC2, 0x3D, 0x0F, 0xF1, 0xEE, 0xCB,
    0x02, 0x31, 0x1D, 0x30, 0xEB, 0x7A, 0x1E, 0xEC, 0xC2, 0xB4, 0x34, 0xDB,
    0x86, 0x3B, 0x24, 0x74, 0xC8, 0xF8, 0x87, 0x09, 0x1F, 0xB4, 0x4F, 0xDC,
    0xEB, 0x83, 0x0A, 0x31, 0xAE, 0x27, 0x30, 0x24, 0x39, 0x93, 0xBE, 0xEB,
    0x85, 0x61, 0x46, 0xA6, 0x76, 0xB0, 0xA1, 0xD3, 0x3B, 0x42, 0x91, 0xE2,
    0x82, 0xB9, 0x8D, 0x4C, 0xE5, 0x22, 0x74, 0x3D, 0x1A, 0x44, 0x98, 0x00,
    0xFF, 0x11, 0x4A, 0x7F, 0x0C, 0x3D, 0x0B, 0x47, 0x35, 0x66, 0x39, 0xE9,
    0x98, 0x20, 0x58, 0xCC, 0xA2, 0xCC, 0xCC, 0x62, 0x29, 0x40, 0xCD, 0x94,
    0x4E, 0xCD, 0x33, 0x31, 0xFF, 0x0A, 0x37, 0x51, 0xE3, 0xCE, 0xFA, 0xCF,
    0x75, 0xC1, 0x10, 0x46, 0x37, 0xB7, 0x88, 0xF1, 0x88, 0xA7, 0x27, 0xD0,
    0x13, 0xC8, 0xF4, 0x84, 0x60, 0x99, 0x9B, 0x7E, 0x77, 0x66, 0x72, 0x95,
    0xF8, 0x66, 0x67, 0x29, 0x14, 0x89, 0x77, 0x66, 0x99, 0xCB, 0x47, 0x1D,
    0x54, 0x31, 0x87, 0x3F, 0x38, 0x70, 0xA1, 0xC5, 0x98, 0x29, 0x57, 0x30,
    0x6A, 0x18, 0x24, 0x7E, 0xD1, 0x1C, 0xC9, 0xB0, 0x8F, 0x3C, 0xA7, 0x1F,
    0x38, 0x77, 0x20, 0x32, 0xCA, 0x16, 0x60, 0xA5, 0x21, 0xB7, 0xED, 0xD0,
    0x6C, 0x49, 0x54, 0x96, 0x41, 0x54, 0x77, 0x68, 0xE1, 0x0F, 0x43, 0xBC,
    0x76, 0x25, 0x11, 0x47, 0xA3, 0x11, 0x4A, 0x23, 0x7D, 0x34, 0xD2, 0x3D,
    0x1E, 0x47, 0x11, 0x1E, 0xEC, 0xB7, 0x76, 0x51, 0xEE, 0xFC, 0x29, 0x59,
    0x77, 0xC4, 0x48, 0xF7, 0x73, 0x51, 0x3A, 0x98, 0x8C, 0x3D, 0xC2, 0x87,
    0x26, 0x42, 0x59, 0xDA, 0x14, 0x4A, 0x22, 0x8B, 0xF0, 0xC2, 0xEE, 0x4C,
    0xC2, 0xB4, 0xCD, 0x04, 0xDB, 0x75, 0xC2, 0x31, 0x87, 0x8B, 0xBE, 0x62,
    0x11, 0x8A, 0x39, 0x98, 0x72, 0xE1, 0x0B, 0x4E, 0x7B, 0x72, 0x3E, 0x43,
    0xEB, 0xC8, 0xA3, 0xBA, 0xEB, 0xDA, 0x14, 0xA2, 0x35, 0xDD, 0x79, 0x48,
    0xEB, 0x87, 0x33, 0x4F, 0xCE, 0x14, 0x74, 0xB7, 0xC2, 0xCC, 0x14, 0xA2,
    0x34, 0xC2, 0x40, 0x91, 0x95, 0x1D, 0x9A, 0x8F, 0xB3, 0x0C, 0x6E, 0xDB,
    0x8E, 0xB3, 0xC8, 0xEE, 0x1E, 0x71, 0x8C, 0x51, 0x82, 0xCC, 0x14, 0xA1,
    0x98, 0xD0, 0x5E, 0xC4, 0x3B, 0x8C, 0x0F, 0xA3, 0xBB, 0x57, 0x57, 0x29,
    0x86, 0x3C, 0xC6, 0x18, 0x99, 0xC5, 0x27, 0x9D, 0xF4, 0x90, 0x94, 0x31,
    0xCC, 0xEF, 0x0A, 0x55, 0xA7, 0x22, 0x77, 0x82, 0xEC, 0x65, 0x26, 0x72,
    0x23, 0x98, 0xBA, 0x38, 0x97, 0xA3, 0xAF, 0x3A, 0x1D, 0xCC, 0xAB, 0xD3,
    0xAF, 0x68, 0x52, 0xA3, 0xF1, 0x1D, 0xAD, 0x8C, 0x47, 0x66, 0x8E, 0x68,
    0x82, 0x31, 0x4C, 0x7A, 0x03, 0x27, 0x77, 0x25, 0x98, 0x29, 0x52, 0x41,
    0xD1, 0x27, 0x7C, 0x47, 0xA1, 0x14, 0x4B, 0x8F, 0x51, 0xF6, 0xD2, 0xC9,
    0x23, 0xE7, 0x0A, 0x37, 0xE3, 0xAD, 0xDD, 0x22, 0x56, 0x9E, 0xD8, 0xF6,
    0xD4, 0xF1, 0x9F, 0x1D, 0xFE, 0x70, 0x9E, 0x4A, 0xAC, 0xB0, 0x22, 0x95,
    0xF8, 0x5B, 0x98, 0x36, 0xE1, 0x98, 0xFB, 0x47, 0x7C, 0xE9, 0xB0, 0x9B,
    0x22, 0x5D, 0xAB, 0x12, 0xE3, 0x28, 0x89, 0x70, 0xA4, 0x48, 0xDD, 0x00,
    0x41, 0x22, 0x54, 0xBB, 0x63, 0xB8, 0x93, 0x18, 0xF0, 0x17, 0x68, 0xBA,
    0xCF, 0x3E, 0xEF, 0x4F, 0xD0, 0x67, 0xCE, 0x8F, 0x79, 0xA2, 0x24, 0x85,
    0x23, 0x6B, 0x07, 0x25, 0x19, 0xC0, 0xB6, 0x7D, 0x44, 0xB9, 0x1C, 0xC2,
    0x3A, 0xF4, 0x8B, 0x6B, 0x36, 0xD3, 0x6B, 0x7F, 0x38, 0x5F, 0x57, 0xA0,
    0x98, 0x83, 0x5E, 0x85, 0x98, 0x29, 0x4F, 0xA0, 0x2D, 0x82, 0xAF, 0xCC,
    0xC4, 0x04, 0xE7, 0xEA, 0x3E, 0xDB, 0x26, 0xD5, 0xF1, 0x26, 0x01, 0xE2,
    0x8E, 0x3C, 0xE8, 0xF8, 0x20, 0xE0, 0xEC, 0x19, 0x82, 0xA0, 0x9E, 0x8F,
    0xB1, 0x14, 0x88, 0x8E, 0xC3, 0x36, 0xD3, 0x1F, 0xA2, 0xC8, 0x3A, 0x11,
    0xF2, 0x83, 0x32, 0xAA, 0x3B, 0xB5, 0x7E, 0x91, 0xD7, 0x0A, 0x31, 0xAE,
    0xD2, 0xFC, 0x44, 0xAD, 0x34, 0x61, 0xCE, 0xB9, 0xCC, 0x68, 0xC2, 0x94,
    0xD9, 0xAA, 0x51, 0x26, 0x27, 0x35, 0x50, 0xA7, 0xCE, 0x17, 0xCF, 0x19,
    0xA1, 0xB4, 0x6D, 0xFA, 0x27, 0x70, 0x7A, 0x4D, 0x51, 0x2E, 0x6B, 0x10,
    0xF7, 0x31, 0x66, 0x0A, 0x51, 0x91, 0xB9, 0x8E, 0xD8, 0xCA, 0xAC, 0xEB,
    0x03, 0x46, 0xE5, 0x81, 0xA3, 0x99, 0x54, 0x55, 0x11, 0xFE, 0x75, 0x2D,
    0x59, 0xD0, 0xB3, 0x11, 0x5E, 0x5A, 0xEF, 0xE2, 0xB0, 0x54, 0x2D, 0xAE,
    0x11, 0x06, 0x34, 0xBE, 0x06, 0x57, 0xDB, 0xE3, 0x81, 0x69, 0xD9, 0x01,
    0xD0, 0xBB, 0x42, 0x2C, 0xA1, 0x60, 0xA1, 0x40, 0xB1, 0x66, 0x03, 0x04,
    0x82, 0xC2, 0xA1, 0x71, 0x48, 0xCC, 0x76, 0x41, 0x22, 0x93, 0x4A, 0x27,
    0x73, 0xDA, 0x05, 0x06, 0x91, 0x4C, 0xA7, 0x56, 0x2C, 0x56, 0x4B, 0x45,
    0xA6, 0xE1, 0x71, 0xB9, 0x5C, 0xEE, 0x97, 0x6B, 0xBD, 0xEE, 0xFB, 0x82,
    0xC5, 0x63, 0x32, 0x99, 0x6C, 0xF6, 0x83, 0x45, 0xA8, 0xD5, 0x6D, 0x37,
    0xDC, 0x0E, 0x0F, 0x23, 0x95, 0xD0, 0xE9, 0x77, 0x3B, 0xDE, 0x2F, 0x37,
    0xBF, 0xED, 0xFD, 0x09, 0x41, 0xE1, 0x10, 0x98, 0x6C, 0x3A, 0x1F, 0x10,
    0x8D, 0x47, 0xA5, 0xD3, 0x09, 0xB4, 0xF2, 0x85, 0x6B, 0xEA, 0x7A, 0x3E,
    0x5F, 0x3F, 0xD4, 0x72, 0x3F, 0x24, 0x9D, 0x4F, 0xE8, 0x96, 0x6B, 0x3D,
    0xBF, 0xF2, 0x11, 0x81, 0xC3, 0x27, 0xD6, 0x3B, 0xCF, 0xD3, 0xF1, 0xFE,
    0xFF, 0x83, 0x01, 0xA0, 0xE8, 0x14, 0x1A, 0x31, 0x34, 0x9C, 0x58, 0x2D,
    0x97, 0x8F, 0xB8, 0x40, 0x11, 0x7C, 0xBF, 0xFF, 0x01, 0x57, 0xEF, 0x80,
    0x18, 0x16, 0x04, 0x01, 0x01, 0xF0, 0x00, 0x90, 0x28, 0x3C, 0x06, 0x01,
    0x00, 0x40, 0x07, 0xC4, 0xE5, 0x42, 0x39, 0x91, 0x25, 0x42, 0x00, 0x03,
};
static const unsigned int kEmbeddedBigfont2Crm2Size = 1104u;

static bool GL_LoadEmbeddedCrm2(const unsigned char* src, unsigned int srcSize, CrmFile& out)
{
	if (out.data)
	{
		std::free(out.data);
		out.data = nullptr;
	}
	out.size = 0;

	if (!src || srcSize == 0)
	{
		return false;
	}

	if (srcSize > 14 && GetSize((void*)src) != 0)
	{
		const unsigned int outSize = GetSize((void*)src);
		const unsigned int headroom = GetSecDist((void*)src);
		unsigned char* indata = static_cast<unsigned char*>(std::malloc(srcSize));
		unsigned char* outdata = static_cast<unsigned char*>(std::malloc(outSize + headroom));
		out.data = static_cast<unsigned char*>(std::malloc(outSize));
		if (!indata || !outdata || !out.data)
		{
			if (indata) std::free(indata);
			if (outdata) std::free(outdata);
			if (out.data) std::free(out.data);
			out.data = nullptr;
			return false;
		}

		std::memcpy(indata, src, srcSize);
		Decrunch(indata, outdata);
		std::memcpy(out.data, outdata, outSize);
		out.size = outSize;
		std::free(indata);
		std::free(outdata);
		return true;
	}

	out.data = static_cast<unsigned char*>(std::malloc(srcSize));
	if (!out.data)
	{
		return false;
	}
	std::memcpy(out.data, src, srcSize);
	out.size = srcSize;
	return true;
}

static bool GL_DecodeEmbeddedPic(const std::uint8_t* src,
                                 std::size_t srcSize,
                                 std::vector<std::uint8_t>& pic,
                                 std::uint32_t& width)
{
    CrmFile picfile;
    if (!GL_LoadEmbeddedCrm2(src, static_cast<unsigned int>(srcSize), picfile) ||
        !picfile.data || picfile.size < 12)
    {
        return false;
    }

    IffHandler::DecodeIff(picfile.data, pic, width);
    return width > 0 && !pic.empty();
}

static bool GL_ApplyPaletteData(const std::uint8_t* data,
                                std::uint32_t size,
                                SDL_Surface* render8)
{
    if (!data || size == 0 || !render8 || !render8->format ||
        !render8->format->palette)
    {
        return false;
    }

    const std::uint32_t numColours = std::min<std::uint32_t>(256, size / 4);
    if (numColours == 0)
    {
        return false;
    }

    for (std::uint32_t c = 0; c < numColours; ++c)
    {
        SDL_Color col;
        col.a = 0xFF;
        col.r = data[c * 4 + 0] & 0x0F;
        col.g = data[c * 4 + 1] >> 4;
        col.b = data[c * 4 + 1] & 0x0F;

        col.r <<= 4;
        col.g <<= 4;
        col.b <<= 4;

        col.r |= data[c * 4 + 2] & 0x0F;
        col.g |= data[c * 4 + 3] >> 4;
        col.b |= data[c * 4 + 3] & 0x0F;

        SDL_SetPaletteColors(render8->format->palette, &col, c, 1);
    }

    return true;
}

static bool GL_ApplyEmbeddedPalette(const std::uint8_t* src,
                                    std::size_t srcSize,
                                    SDL_Surface* render8)
{
    CrmFile palfile;
    if (!GL_LoadEmbeddedCrm2(src, static_cast<unsigned int>(srcSize), palfile) ||
        !palfile.data || palfile.size == 0)
    {
        return false;
    }

    return GL_ApplyPaletteData(palfile.data, palfile.size, render8);
}

static bool GL_CopyDecodedPicToSurface(const std::vector<std::uint8_t>& pic,
                                       std::uint32_t width,
                                       SDL_Surface* render8,
                                       int dstY = 0,
                                       bool clearSurface = true)
{
    if (!render8 || width == 0 || pic.empty() || dstY >= render8->h)
    {
        return false;
    }

    if (clearSurface)
    {
        SDL_FillRect(render8, nullptr, 0);
    }

    const std::uint32_t height = static_cast<std::uint32_t>(pic.size() / width);
    const std::uint32_t copyW = std::min<std::uint32_t>(width,
        static_cast<std::uint32_t>(render8->w));
    const std::uint32_t copyH = std::min<std::uint32_t>(height,
        static_cast<std::uint32_t>(std::max(0, render8->h - dstY)));

    for (std::uint32_t y = 0; y < copyH; ++y)
    {
        const std::uint8_t* src = pic.data() + y * width;
        std::uint8_t* dst = static_cast<std::uint8_t*>(render8->pixels) +
            (dstY + static_cast<int>(y)) * render8->pitch;
        std::copy(src, src + copyW, dst);
    }

    return true;
}



Uint32 my_callbackfunc(Uint32 interval, void *param)
{
	SDL_Event event;
	SDL_UserEvent userevent;

	/* In this example, our callback pushes an SDL_USEREVENT event
	into the queue, and causes our callback to be called again at the
	same interval: */

	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = NULL;
	userevent.data2 = NULL;

	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);
	return(interval);
}

static void fill_audio(void *udata, Uint8 *stream, int len)
{
	auto res = xmp_play_buffer((xmp_context)udata, stream, len, 0);
}

static bool DecodePicFile(const std::string& name, std::vector<uint8_t>& pic, uint32_t& width)
{
	CrmFile picfile;
	if (!picfile.Load(name.c_str()) || !picfile.data || picfile.size < 12)
	{
		SDL_Log("ZGloom: DecodePicFile('%s') failed", name.c_str());
		return false;
	}

	IffHandler::DecodeIff(picfile.data, pic, width);
	return (width > 0) && !pic.empty();
}

static bool ApplyPicPalette(const std::string& name, SDL_Surface* render8)
{
	CrmFile palfile;
	if (!palfile.Load((name + ".pal").c_str()) || !palfile.data || palfile.size == 0)
	{
		SDL_Log("ZGloom: ApplyPicPalette('%s') failed", name.c_str());
		return false;
	}

	return GL_ApplyPaletteData(palfile.data, palfile.size, render8);
}

bool LoadPic(std::string name, SDL_Surface* render8)
{
	if (!render8)
	{
		return false;
	}

	std::vector<uint8_t> pic;
	uint32_t width = 0;
	if (!DecodePicFile(name, pic, width))
	{
		SDL_FillRect(render8, nullptr, 0);
		return false;
	}

	ApplyPicPalette(name, render8);
	return GL_CopyDecodedPicToSurface(pic, width, render8);
}

static bool GL_LoadPicWithEmbeddedFallback(const std::string& name,
                                           SDL_Surface* render8,
                                           const std::uint8_t* embeddedPic,
                                           std::size_t embeddedPicSize,
                                           const std::uint8_t* embeddedPalette,
                                           std::size_t embeddedPaletteSize)
{
    if (!render8)
    {
        return false;
    }

    std::vector<std::uint8_t> pic;
    std::uint32_t width = 0;
    const bool loadedFromDisk = DecodePicFile(name, pic, width);

    if (!loadedFromDisk &&
        !GL_DecodeEmbeddedPic(embeddedPic, embeddedPicSize, pic, width))
    {
        SDL_FillRect(render8, nullptr, 0);
        SDL_Log("ZGloom: embedded fallback for '%s' also failed", name.c_str());
        return false;
    }

    bool paletteLoaded = false;
    if (loadedFromDisk)
    {
        paletteLoaded = ApplyPicPalette(name, render8);
    }
    if (!paletteLoaded)
    {
        paletteLoaded = GL_ApplyEmbeddedPalette(embeddedPalette,
                                                embeddedPaletteSize,
                                                render8);
        if (paletteLoaded)
        {
            SDL_Log("ZGloom: using embedded palette fallback for '%s'", name.c_str());
        }
    }

    if (!loadedFromDisk)
    {
        SDL_Log("ZGloom: using embedded picture fallback for '%s'", name.c_str());
    }

    return GL_CopyDecodedPicToSurface(pic, width, render8);
}

static bool OverlayPicAt(const std::string& name, SDL_Surface* render8, int dstY)
{
	if (!render8 || dstY >= render8->h)
	{
		return false;
	}

	std::vector<uint8_t> pic;
	uint32_t width = 0;
	if (!DecodePicFile(name, pic, width))
	{
		return false;
	}

	return GL_CopyDecodedPicToSurface(pic, width, render8, dstY, false);
}

static bool GL_OverlayEmbeddedPicAt(const std::uint8_t* embeddedPic,
                                    std::size_t embeddedPicSize,
                                    SDL_Surface* render8,
                                    int dstY)
{
    std::vector<std::uint8_t> pic;
    std::uint32_t width = 0;
    if (!GL_DecodeEmbeddedPic(embeddedPic, embeddedPicSize, pic, width))
    {
        return false;
    }

    return GL_CopyDecodedPicToSurface(pic, width, render8, dstY, false);
}

// Present a 320-pixel static screen in widescreen using the same principle as
// gloom2.s c87w1: keep the original picture untouched in the centre and extend
// only the first/last pixel of each scanline into a four-step dark edge wash.
// This avoids the visibly smeared 16-pixel side strips used by older Android
// builds and never stretches menu/intermission text into the side areas.
static void GL_BlitStaticWideLikeGloom2(SDL_Surface* source32,
                                        SDL_Surface* destination32,
                                        const SDL_Rect& centre)
{
    if (!source32 || !destination32 ||
        source32->format->BytesPerPixel != 4 || destination32->format->BytesPerPixel != 4)
    {
        if (source32 && destination32)
            SDL_BlitScaled(source32, nullptr, destination32, const_cast<SDL_Rect*>(&centre));
        return;
    }

    const int leftWidth = std::max(0, centre.x);
    const int rightStart = centre.x + centre.w;
    const int rightWidth = std::max(0, destination32->w - rightStart);

    if (SDL_MUSTLOCK(source32) && SDL_LockSurface(source32) != 0)
        return;
    if (SDL_MUSTLOCK(destination32) && SDL_LockSurface(destination32) != 0)
    {
        if (SDL_MUSTLOCK(source32)) SDL_UnlockSurface(source32);
        return;
    }

    for (int dy = 0; dy < centre.h; ++dy)
    {
        const int dstY = centre.y + dy;
        if (dstY < 0 || dstY >= destination32->h)
            continue;

        const int srcY = std::min(source32->h - 1,
                                  std::max(0, (dy * source32->h) / std::max(1, centre.h)));
        const uint32_t* srcRow = reinterpret_cast<const uint32_t*>(
            static_cast<const uint8_t*>(source32->pixels) + srcY * source32->pitch);
        uint32_t* dstRow = reinterpret_cast<uint32_t*>(
            static_cast<uint8_t*>(destination32->pixels) + dstY * destination32->pitch);

        Uint8 lr = 0, lg = 0, lb = 0, la = 255;
        Uint8 rr = 0, rg = 0, rb = 0, ra = 255;
        SDL_GetRGBA(srcRow[0], source32->format, &lr, &lg, &lb, &la);
        SDL_GetRGBA(srcRow[source32->w - 1], source32->format, &rr, &rg, &rb, &ra);

        uint32_t leftShades[4];
        uint32_t rightShades[4];
        for (int shade = 0; shade < 4; ++shade)
        {
            const int numerator = shade + 1; // quarter, half, three-quarter, full
            leftShades[shade] = SDL_MapRGBA(destination32->format,
                static_cast<Uint8>((static_cast<int>(lr) * numerator) / 4),
                static_cast<Uint8>((static_cast<int>(lg) * numerator) / 4),
                static_cast<Uint8>((static_cast<int>(lb) * numerator) / 4), 255);
            rightShades[shade] = SDL_MapRGBA(destination32->format,
                static_cast<Uint8>((static_cast<int>(rr) * numerator) / 4),
                static_cast<Uint8>((static_cast<int>(rg) * numerator) / 4),
                static_cast<Uint8>((static_cast<int>(rb) * numerator) / 4), 255);
        }

        for (int x = 0; x < leftWidth; ++x)
        {
            const int shade = std::min(3, (x * 4) / std::max(1, leftWidth));
            dstRow[x] = leftShades[shade];
        }
        for (int x = 0; x < rightWidth; ++x)
        {
            const int shade = std::min(3,
                ((rightWidth - 1 - x) * 4) / std::max(1, rightWidth));
            dstRow[rightStart + x] = rightShades[shade];
        }
    }

    if (SDL_MUSTLOCK(destination32)) SDL_UnlockSurface(destination32);
    if (SDL_MUSTLOCK(source32)) SDL_UnlockSurface(source32);

    SDL_BlitScaled(source32, nullptr, destination32, const_cast<SDL_Rect*>(&centre));
}

// Gloom Classic deliberately uses a coarser software look.  The world is still
// rendered normally for compatibility, then reduced to stable nearest-neighbour
// 2x2 blocks before the separate HUD layer is composed.  Menus never pass here.
static void GL_PixelateWorld2x2(SDL_Surface* surface)
{
    if (!surface || surface->format->BytesPerPixel != 4)
        return;

    if (SDL_MUSTLOCK(surface) && SDL_LockSurface(surface) != 0)
        return;

    for (int y = 0; y < surface->h; y += 2)
    {
        uint32_t* row0 = reinterpret_cast<uint32_t*>(
            static_cast<uint8_t*>(surface->pixels) + y * surface->pitch);
        uint32_t* row1 = (y + 1 < surface->h)
            ? reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(surface->pixels) + (y + 1) * surface->pitch)
            : row0;

        for (int x = 0; x < surface->w; x += 2)
        {
            const uint32_t pixel = row0[x];
            row0[x] = pixel;
            if (x + 1 < surface->w) row0[x + 1] = pixel;
            row1[x] = pixel;
            if (x + 1 < surface->w) row1[x + 1] = pixel;
        }
    }

    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
}


enum GameState
{
	STATE_PLAYING,
	STATE_PARSING,
	STATE_SPOOLING,
	STATE_WAITING,
	STATE_MENU,
	STATE_SPLASH,
	STATE_TITLE
};

bool g_RequestSavePosition  = false;
bool g_RequestTitleContinue = false;




int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    SDL_SetHint("ZGLOOM_COLLECTION_CREDIT", kZGloomCollectionCredit);

    sceClibPrintf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    char liveAreaBuffer[2048];
    std::memset(liveAreaBuffer, 0, sizeof(liveAreaBuffer));

    SceAppUtilInitParam initParam = {};
    SceAppUtilBootParam bootParam = {};
    sceAppUtilInit(&initParam, &bootParam);
    SceAppUtilAppEventParam eventParam;
    sceClibMemset(&eventParam, 0, sizeof(eventParam));
    sceAppUtilReceiveAppEvent(&eventParam);
    if (eventParam.type == 0x05)
    {
        sceAppUtilAppEventParseLiveArea(&eventParam, liveAreaBuffer);
        if (std::strstr(liveAreaBuffer, "deluxe")) Config::SetGame(Config::DELUXE);
        else if (std::strstr(liveAreaBuffer, "gloom3")) Config::SetGame(Config::GLOOM3);
        else if (std::strstr(liveAreaBuffer, "massacre")) Config::SetGame(Config::MASSACRE);
        else Config::SetGame(Config::GLOOM);
    }
    else
    {
        Config::SetGame(Config::GLOOM);
    }

    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
    {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    Config::Init();

    // Preserve the Vita-specific pre-boot game selector and ux0: data layout.
    {
        const std::string selected = bootselect::Run(nullptr);
        if (!selected.empty())
        {
            Config::GameTitle game = Config::GLOOM;
            if (selected == "deluxe") game = Config::DELUXE;
            else if (selected == "gloom3") game = Config::GLOOM3;
            else if (selected == "massacre") game = Config::MASSACRE;
            Config::SetGame(game);
            Config::SetZM(game == Config::MASSACRE);
            Config::Init();
        }
    }

    const int gameId = Config::GetGameID();
    const bool selectedGloomClassic = (gameId == Config::GLOOM);
    const bool selectedGloom3 = (gameId == Config::GLOOM3);
    const bool selectedZombieMassacre = (gameId == Config::MASSACRE);
    Config::SetZM(selectedZombieMassacre);

    const int gameDir = sceIoDopen(Config::GetGamePath().c_str());
    if (gameDir < 0)
    {
        SDL_Quit();
        return 0;
    }
    sceIoDclose(gameDir);

    AtmosphereVolume::LoadFromConfig();
    BGM::Init();
    BGM::SetVolume9(AtmosphereVolume::Get());

    GloomMap gmap;
    Script script;
    TitleScreen titlescreen;
    MenuScreen menuscreen;
    GameState state = STATE_TITLE;
    xmp_context ctx = xmp_create_context();
    Config::RegisterMusContext(ctx);

    int renderwidth = 320, renderheight = 256, windowwidth = 960, windowheight = 544;
    Config::GetRenderSizes(renderwidth, renderheight, windowwidth, windowheight);

    CrmFile titlemusic;
    CrmFile intermissionmusic;
    CrmFile ingamemusic;
    titlemusic.Load(Config::GetMusicFilename(0).c_str());
    intermissionmusic.Load(Config::GetMusicFilename(1).c_str());

    SoundHandler::Init();

    SDL_Window* win = SDL_CreateWindow("ZGloom", 100, 100, windowwidth, windowheight,
        SDL_WINDOW_SHOWN | (Config::GetFullscreen() ? SDL_WINDOW_FULLSCREEN : 0));
    if (!win)
    {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    Config::RegisterWin(win);

    SDL_Renderer* ren = nullptr; // Vita presentation is handled by vita2d.
    RendererHooks::init(ren, windowwidth, windowheight);
    RendererHooks::setTargetFps(Config::GetMaxFps());
    RendererHooks::setRenderSize(renderwidth, renderheight);
    SDL_ShowCursor(SDL_DISABLE);

    SDL_Surface* render8 = SDL_CreateRGBSurface(0, 320, 256, 8, 0, 0, 0, 0);
    SDL_Surface* intermissionscreen = SDL_CreateRGBSurface(0, 320, 256, 8, 0, 0, 0, 0);
    SDL_Surface* titlebitmap = SDL_CreateRGBSurface(0, 320, 256, 8, 0, 0, 0, 0);
    SDL_Surface* titlemenubitmap = SDL_CreateRGBSurface(0, 320, 256, 8, 0, 0, 0, 0);
    SDL_Surface* splashbitmap = SDL_CreateRGBSurface(0, 320, 256, 8, 0, 0, 0, 0);
    SDL_Surface* render32 = SDL_CreateRGBSurface(0, renderwidth, renderheight, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_Surface* screen32 = SDL_CreateRGBSurface(0, 320, 256, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_Surface* zmTitleOverlay8 = nullptr;
    SDL_Surface* zmTitleOverlay32 = nullptr;

    if (!render8 || !intermissionscreen || !titlebitmap || !titlemenubitmap ||
        !splashbitmap || !render32 || !screen32)
    {
        SDL_Log("ZGloom Vita: surface allocation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    ObjectGraphics objgraphics;
    Renderer renderer;
    GameLogic logic;
    Camera cam;
    Hud hud;
    logic.Init(&objgraphics);
    SDL_AddTimer(1000 / 25, my_callbackfunc, nullptr);

    Font smallfont, bigfont;
    CrmFile fontfile;
    if (GL_LoadEmbeddedCrm2(kEmbeddedBigfont2Crm2, kEmbeddedBigfont2Crm2Size, fontfile))
    {
        bigfont.Load2(fontfile);
        smallfont.Load2(fontfile);
    }
    else
    {
        fontfile.Load((Config::GetMiscDir() + "bigfont2.bin").c_str());
        if (fontfile.data)
        {
            bigfont.Load2(fontfile);
            smallfont.Load2(fontfile);
        }
        else
        {
            fontfile.Load((Config::GetMiscDir() + "smallfont.bin").c_str());
            if (fontfile.data) smallfont.Load(fontfile);
            fontfile.Load((Config::GetMiscDir() + "bigfont.bin").c_str());
            if (fontfile.data) bigfont.Load(fontfile);
        }
    }

    const bool haveSplash = LoadPic(Config::GetPicsDir() + "blackmagic", splashbitmap);
    const bool haveTitle = selectedGloomClassic
        ? GL_LoadPicWithEmbeddedFallback(
            Config::GetPicsDir() + "title", titlebitmap,
            kEmbeddedGloomClassicTitleCrm2, kEmbeddedGloomClassicTitleCrm2Size,
            kEmbeddedGloomClassicTitlePal, kEmbeddedGloomClassicTitlePalSize)
        : LoadPic(Config::GetPicsDir() + "title", titlebitmap);

    if (haveTitle)
    {
        SDL_SetPaletteColors(titlemenubitmap->format->palette,
            titlebitmap->format->palette->colors, 0, 256);
        SDL_BlitSurface(titlebitmap, nullptr, titlemenubitmap, nullptr);

        // Gloom 3 and Zombie Massacre already provide their complete main title.
        if (!selectedGloom3 && !selectedZombieMassacre)
        {
            if (selectedGloomClassic)
            {
                if (!OverlayPicAt(Config::GetPicsDir() + "gloombrush", titlemenubitmap, 168) &&
                    !OverlayPicAt(Config::GetPicsDir() + "gloom", titlemenubitmap, 168))
                {
                    GL_OverlayEmbeddedPicAt(kEmbeddedGloomClassicBrushCrm2,
                        kEmbeddedGloomClassicBrushCrm2Size, titlemenubitmap, 168);
                }
            }
            else if (!OverlayPicAt(Config::GetPicsDir() + "gloom", titlemenubitmap, 168))
            {
                OverlayPicAt(Config::GetPicsDir() + "gloombrush", titlemenubitmap, 168);
            }
        }
    }
    else if (haveSplash)
    {
        SDL_SetPaletteColors(titlebitmap->format->palette,
            splashbitmap->format->palette->colors, 0, 256);
        SDL_BlitSurface(splashbitmap, nullptr, titlebitmap, nullptr);
        SDL_SetPaletteColors(titlemenubitmap->format->palette,
            splashbitmap->format->palette->colors, 0, 256);
        SDL_BlitSurface(splashbitmap, nullptr, titlemenubitmap, nullptr);
    }
    else
    {
        SDL_FillRect(titlebitmap, nullptr, 0);
        SDL_FillRect(titlemenubitmap, nullptr, 0);
    }

    // Zombie Massacre: always prefer the canonical embedded g3-dc brush and
    // palette.  This also overrides stale data files from older Vita releases.
    // The disk file remains a last-resort fallback.  Draw position stays Y=167.
    if (selectedZombieMassacre)
    {
        zmTitleOverlay8 = SDL_CreateRGBSurface(0, 320, 70, 8, 0, 0, 0, 0);
        bool loadedZmBrush = false;
        if (zmTitleOverlay8)
        {
            std::vector<std::uint8_t> embeddedPic;
            std::uint32_t embeddedWidth = 0;
            if (GL_DecodeEmbeddedPic(kEmbeddedZombieMassacreG3Dc,
                    kEmbeddedZombieMassacreG3DcSize,
                    embeddedPic, embeddedWidth))
            {
                GL_ApplyEmbeddedPalette(kEmbeddedZombieMassacreG3DcPal,
                    kEmbeddedZombieMassacreG3DcPalSize, zmTitleOverlay8);
                loadedZmBrush = GL_CopyDecodedPicToSurface(
                    embeddedPic, embeddedWidth, zmTitleOverlay8);
            }
            if (!loadedZmBrush)
                loadedZmBrush = LoadPic(Config::GetPicsDir() + "g3-dc",
                    zmTitleOverlay8);
        }
        if (loadedZmBrush)
        {
            zmTitleOverlay32 = SDL_ConvertSurfaceFormat(zmTitleOverlay8,
                SDL_PIXELFORMAT_ARGB8888, 0);
            if (zmTitleOverlay32)
                SDL_SetSurfaceBlendMode(zmTitleOverlay32, SDL_BLENDMODE_NONE);
        }
    }

    if (haveSplash && haveTitle) state = STATE_SPLASH;
    const uint32_t splashStartTicks = SDL_GetTicks();
    const uint32_t splashDurationMs = 1200;

    if (titlemusic.data)
    {
        if (xmp_load_module_from_memory(ctx, titlemusic.data, titlemusic.size))
            std::cout << "music error";
        if (xmp_start_player(ctx, 44100, 0))
            std::cout << "music error";
        Mix_HookMusic(fill_audio, ctx);
        Config::SetMusicVol(Config::GetMusicVol());
    }

    std::string intermissiontext;
    std::size_t intermissionTypewriterTotal = 0;
    uint32_t intermissionTypewriterStartTicks = 0;
    bool intermissionTypewriterComplete = true;
    const uint32_t intermissionTypewriterCharacterMs = 40;
    bool skipIntermissionForResume = false;
    bool resumeRequested = false;
    bool intermissionPictureValid = false;
    bool intermissionmusplaying = false;
    bool haveingamemusic = false;
    bool printscreen = false;
    int screennum = 0;
    uint32_t fps = 0;
    uint32_t fpscounter = 0;

    Mix_Volume(-1, Config::GetSFXVol() * 12);
    Mix_VolumeMusic(Config::GetMusicVol() * 12);

    SDL_Rect blitrect;
    const int screenscale = std::max(1, renderheight / 256);
    blitrect.w = 320 * screenscale;
    blitrect.h = 256 * screenscale;
    blitrect.x = (renderwidth - blitrect.w) / 2;
    blitrect.y = (renderheight - blitrect.h) / 2;

    const std::vector<LevelDescriptor>& levelcatalog = script.GetLevels();
    titlescreen.SetLevels(levelcatalog);
    int levelselect = 0;

    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    Input::Init();
    SDL_Event sEvent;
    bool notdone = true;

    while (notdone)
    {
        Input::Update();
        RendererHooks::beginFrame();

        if ((state == STATE_PARSING) || (state == STATE_SPOOLING))
        {
            std::string scriptstring;
            const Script::ScriptOp sop = script.NextLine(scriptstring);
            switch (sop)
            {
                case Script::SOP_SETPICT:
                    scriptstring.insert(0, Config::GetPicsDir());
                    intermissionPictureValid = LoadPic(scriptstring, intermissionscreen);
                    if (intermissionPictureValid)
                    {
                        SDL_SetPaletteColors(render8->format->palette,
                            intermissionscreen->format->palette->colors, 0, 256);
                    }
                    break;

                case Script::SOP_SONG:
                    scriptstring.insert(0, Config::GetMusicDir());
                    ingamemusic.Load(scriptstring.c_str());
                    haveingamemusic = (ingamemusic.data != nullptr);
                    break;

                case Script::SOP_LOADFLAT:
                {
                    char* end = nullptr;
                    const long parsedFlat = std::strtol(scriptstring.c_str(), &end, 10);
                    while (end && *end && std::isspace(static_cast<unsigned char>(*end))) ++end;
                    if (end != scriptstring.c_str() && end && *end == '\0' &&
                        parsedFlat >= 0 && parsedFlat <= 999)
                    {
                        if (!gmap.SetFlat(static_cast<int>(parsedFlat)))
                            SDL_Log("ZGloom Vita: failed to load flat %ld", parsedFlat);
                    }
                    break;
                }

                case Script::SOP_LOADMAP:
                case Script::SOP_NOP:
                    break;

                case Script::SOP_TEXT:
                    intermissiontext = scriptstring;
                    break;

                case Script::SOP_DRAW:
                    if (state == STATE_PARSING && !skipIntermissionForResume && intermissionmusic.data)
                    {
                        if (xmp_load_module_from_memory(ctx, intermissionmusic.data, intermissionmusic.size))
                            std::cout << "music error";
                        if (xmp_start_player(ctx, 44100, 0))
                            std::cout << "music error";
                        Mix_HookMusic(fill_audio, ctx);
                        Config::SetMusicVol(Config::GetMusicVol());
                        intermissionmusplaying = true;
                    }
                    break;

                case Script::SOP_WAIT:
                    if (state == STATE_PARSING)
                    {
                        if (skipIntermissionForResume && resumeRequested)
                        {
                            intermissiontext.clear();
                            intermissionTypewriterTotal = 0;
                            intermissionTypewriterComplete = true;
                            break;
                        }
                        state = STATE_WAITING;
                        intermissionTypewriterTotal = smallfont.CountTypewriterCharacters(intermissiontext);
                        intermissionTypewriterStartTicks = SDL_GetTicks();
                        intermissionTypewriterComplete = (intermissionTypewriterTotal == 0);
                        SDL_SetPaletteColors(render8->format->palette,
                            smallfont.GetPalette()->colors, 0, 16);
                        SDL_BlitSurface(intermissionscreen, nullptr, render8, nullptr);
                    }
                    break;

                case Script::SOP_PLAY:
                    if (state == STATE_PARSING)
                    {
                        std::string levelRel = scriptstring;
                        SaveSystem::SaveData saved;
                        bool haveSavePos = false;
                        bool haveReplay = false;
                        EventReplay::Clear();

                        if (resumeRequested)
                        {
                            resumeRequested = false;
                            skipIntermissionForResume = false;
                            if (SaveSystem::LoadFromDisk(saved))
                            {
                                levelRel = saved.levelPath;
                                const std::string levelPrefix = Config::GetLevelDir();
                                if (levelRel.compare(0, levelPrefix.size(), levelPrefix) == 0)
                                    levelRel.erase(0, levelPrefix.size());
                                haveSavePos = true;

                                int scriptedFlat = -1;
                                if (script.GetFlatForLevel(levelRel, scriptedFlat))
                                {
                                    if (saved.formatVersion == 1 || saved.flatIndex < 0)
                                        saved.flatIndex = scriptedFlat;
                                }
                                script.SeekAfterPlayFor(levelRel);

                                if (saved.formatVersion >= 2)
                                {
                                    EventReplay::SetEvents(saved.eventHistory);
                                    haveReplay = !EventReplay::GetEvents().empty();
                                }
                                else if (EventReplay::LoadFromDisk())
                                {
                                    haveReplay = true;
                                }
                            }
                        }

                        SaveSystem::SetCurrentLevelPath(levelRel);
                        cam.x.SetInt(0);
                        cam.y = 120;
                        cam.z.SetInt(0);
                        cam.rotquick.SetInt(0);

                        const std::string levelFull = Config::GetLevelDir() + levelRel;
                        gmap.Load(levelFull.c_str(), &objgraphics);
                        if (haveSavePos && saved.flatIndex >= 0)
                            gmap.SetFlat(saved.flatIndex);
                        if (haveSavePos)
                            logic.SetLives(saved.lives);

                        renderer.Init(render32, &gmap, &objgraphics);
                        logic.InitLevel(&gmap, &cam, &objgraphics);

                        if (haveReplay)
                        {
                            EventReplay::ReplayAll(gmap);
                            logic.RestoreTriggeredEvents(EventReplay::GetEvents());
                        }
                        if (haveSavePos)
                            SaveSystem::ApplyToGame(saved, cam, gmap);

                        state = STATE_PLAYING;
                        BGM::PlayLooping();
                        BGM::SetVolume9(AtmosphereVolume::Get());
                        EmbeddedBGMVolume::ApplyFromConfig();

                        if (haveingamemusic)
                        {
                            if (xmp_load_module_from_memory(ctx, ingamemusic.data, ingamemusic.size))
                                std::cout << "music error";
                            if (xmp_start_player(ctx, 44100, 0))
                                std::cout << "music error";
                            Mix_HookMusic(fill_audio, ctx);
                            Config::SetMusicVol(Config::GetMusicVol());
                        }
                    }
                    break;

                case Script::SOP_END:
                    BGM::Stop();
                    state = STATE_TITLE;
                    if (intermissionmusic.data && intermissionmusplaying)
                    {
                        Mix_HookMusic(nullptr, nullptr);
                        xmp_end_player(ctx);
                        xmp_release_module(ctx);
                        intermissionmusplaying = false;
                    }
                    if (titlemusic.data)
                    {
                        if (xmp_load_module_from_memory(ctx, titlemusic.data, titlemusic.size))
                            std::cout << "music error";
                        if (xmp_start_player(ctx, 44100, 0))
                            std::cout << "music error";
                        Mix_HookMusic(fill_audio, ctx);
                        Config::SetMusicVol(Config::GetMusicVol());
                    }
                    break;
            }
        }

        if (state == STATE_SPLASH)
        {
            SDL_SetPaletteColors(render8->format->palette,
                splashbitmap->format->palette->colors, 0, 256);
            SDL_BlitSurface(splashbitmap, nullptr, render8, nullptr);
            if (SDL_GetTicks() - splashStartTicks >= splashDurationMs ||
                Input::GetButtonDown(SCE_CTRL_CROSS) || Input::GetButtonDown(SCE_CTRL_CIRCLE))
            {
                state = STATE_TITLE;
            }
        }
        else if (state == STATE_TITLE)
        {
            SDL_Surface* titleSource = titlescreen.WantsPlainTitleBackground()
                ? titlebitmap : titlemenubitmap;
            SDL_SetPaletteColors(render8->format->palette,
                titleSource->format->palette->colors, 0, 256);
            titlescreen.Render(titleSource, render8, smallfont);

            switch (titlescreen.Update(levelselect))
            {
                case TitleScreen::TITLERET_PLAY:
                    resumeRequested = false;
                    skipIntermissionForResume = false;
                    EventReplay::Clear();
                    script.Reset();
                    state = STATE_PARSING;
                    BGM::Stop();
                    logic.Init(&objgraphics);
                    if (titlemusic.data)
                    {
                        Mix_HookMusic(nullptr, nullptr);
                        xmp_end_player(ctx);
                        xmp_release_module(ctx);
                    }
                    break;

                case TitleScreen::TITLERET_RESUME:
                    resumeRequested = true;
                    skipIntermissionForResume = true;
                    script.Reset();
                    intermissiontext.clear();
                    state = STATE_PARSING;
                    BGM::Stop();
                    logic.Init(&objgraphics);
                    if (titlemusic.data)
                    {
                        Mix_HookMusic(nullptr, nullptr);
                        xmp_end_player(ctx);
                        xmp_release_module(ctx);
                    }
                    break;

                case TitleScreen::TITLERET_SELECT:
                    if (levelselect >= 0 && levelselect < static_cast<int>(levelcatalog.size()) &&
                        script.SeekToLevel(static_cast<std::size_t>(levelselect)))
                    {
                        const LevelDescriptor& selectedLevel =
                            levelcatalog[static_cast<std::size_t>(levelselect)];
                        resumeRequested = false;
                        skipIntermissionForResume = false;
                        EventReplay::Clear();
                        intermissiontext.clear();
                        if (!selectedLevel.pictureName.empty())
                        {
                            intermissionPictureValid = LoadPic(
                                Config::GetPicsDir() + selectedLevel.pictureName,
                                intermissionscreen);
                        }
                        if (selectedLevel.flatIndex >= 0)
                            gmap.SetFlat(selectedLevel.flatIndex);
                        state = STATE_PARSING;
                        logic.Init(&objgraphics);
                        BGM::Stop();
                        if (titlemusic.data)
                        {
                            Mix_HookMusic(nullptr, nullptr);
                            xmp_end_player(ctx);
                            xmp_release_module(ctx);
                        }
                    }
                    break;

                case TitleScreen::TITLERET_QUIT:
                    notdone = false;
                    break;

                default:
                    break;
            }
        }
        else if (state == STATE_WAITING)
        {
            if (intermissionPictureValid)
            {
                SDL_SetPaletteColors(render8->format->palette,
                    intermissionscreen->format->palette->colors, 0, 256);
            }
            SDL_SetPaletteColors(render8->format->palette,
                smallfont.GetPalette()->colors, 0, 16);
            SDL_BlitSurface(intermissionscreen, nullptr, render8, nullptr);

            std::size_t visibleCharacters = intermissionTypewriterTotal;
            if (!intermissionTypewriterComplete)
            {
                const uint32_t elapsed = SDL_GetTicks() - intermissionTypewriterStartTicks;
                visibleCharacters = std::min<std::size_t>(intermissionTypewriterTotal,
                    static_cast<std::size_t>(elapsed / intermissionTypewriterCharacterMs) + 1);
                if (visibleCharacters >= intermissionTypewriterTotal)
                    intermissionTypewriterComplete = true;
            }
            smallfont.PrintMultiLineMessageProgressive(
                intermissiontext, 220, render8, visibleCharacters);

            if (Input::GetButtonDown(SCE_CTRL_CROSS))
            {
                if (!intermissionTypewriterComplete)
                {
                    intermissionTypewriterComplete = true;
                }
                else
                {
                    state = STATE_PARSING;
                    if (intermissionmusic.data)
                    {
                        Mix_HookMusic(nullptr, nullptr);
                        xmp_end_player(ctx);
                        xmp_release_module(ctx);
                        intermissionmusplaying = false;
                    }
                }
            }
        }

        // Vita pause/menu controls remain native: START toggles, CIRCLE backs out.
        if (state == STATE_PLAYING && Input::GetButtonDown(SCE_CTRL_START))
        {
            menuscreen.ResetToMain();
            state = STATE_MENU;
        }
        else if (state == STATE_MENU)
        {
            if (Input::GetButtonDown(SCE_CTRL_START))
            {
                state = STATE_PLAYING;
            }
            else
            {
                const MenuScreen::MenuReturn mr = menuscreen.Update();
                if (mr == MenuScreen::MENURET_SAVE)
                {
                    if (logic.CanSavePosition())
                    {
                        SaveSystem::SaveData saved;
                        saved.levelPath = SaveSystem::GetCurrentLevelPath();
                        saved.flatIndex = SaveSystem::GetCurrentFlat();
                        saved.camX = cam.x.GetInt();
                        saved.camY = cam.y;
                        saved.camZ = cam.z.GetInt();
                        saved.camRot = cam.rotquick.GetInt();
                        const MapObject player = logic.GetPlayerObj();
                        saved.hp = player.data.ms.hitpoints;
                        saved.lives = logic.GetLives();
                        saved.weapon = player.data.ms.weapon;
                        saved.reload = player.data.ms.reload;
                        saved.reloadcnt = player.data.ms.reloadcnt;
                        saved.eventHistory = EventReplay::GetEvents();
                        if (SaveSystem::SaveToDisk(saved))
                        {
                            EventReplay::SaveToDisk();
                            menuscreen.ResetToMain();
                            state = STATE_PLAYING;
                        }
                    }
                }
                else if (mr == MenuScreen::MENURET_PLAY)
                {
                    state = STATE_PLAYING;
                }
                else if (mr == MenuScreen::MENURET_QUIT)
                {
                    script.Reset();
                    state = STATE_TITLE;
                    BGM::Stop();
                    titlescreen.ResetToMain();
                    if (titlemusic.data)
                    {
                        if (xmp_load_module_from_memory(ctx, titlemusic.data, titlemusic.size))
                            std::cout << "music error";
                        if (xmp_start_player(ctx, 44100, 0))
                            std::cout << "music error";
                        Mix_HookMusic(fill_audio, ctx);
                        Config::SetMusicVol(Config::GetMusicVol());
                    }
                }
            }
        }

        if (state == STATE_PLAYING && Input::GetButtonDown(SCE_CTRL_SELECT))
            Config::SetDebug(!Config::GetDebug());

        while ((state != STATE_SPOOLING) && SDL_PollEvent(&sEvent))
        {
            if (sEvent.type == SDL_WINDOWEVENT && sEvent.window.event == SDL_WINDOWEVENT_CLOSE)
                notdone = false;

            if (sEvent.type == SDL_USEREVENT)
            {
                if (state == STATE_PLAYING)
                {
                    const bool levelFinished = logic.Update(&cam);
                    if (logic.ConsumeGameOverRequest())
                    {
                        script.Reset();
                        EventReplay::Clear();
                        resumeRequested = false;
                        titlescreen.ResetToMain();
                        BGM::Stop();
                        if (haveingamemusic)
                        {
                            Mix_HookMusic(nullptr, nullptr);
                            xmp_end_player(ctx);
                            xmp_release_module(ctx);
                            intermissionmusplaying = false;
                        }
                        state = STATE_TITLE;
                        if (titlemusic.data)
                        {
                            if (xmp_load_module_from_memory(ctx, titlemusic.data, titlemusic.size))
                                std::cout << "music error";
                            if (xmp_start_player(ctx, 44100, 0))
                                std::cout << "music error";
                            Mix_HookMusic(fill_audio, ctx);
                            Config::SetMusicVol(Config::GetMusicVol());
                        }
                    }
                    else if (levelFinished)
                    {
                        BGM::Stop();
                        if (haveingamemusic)
                        {
                            Mix_HookMusic(nullptr, nullptr);
                            xmp_end_player(ctx);
                            xmp_release_module(ctx);
                            intermissionmusplaying = false;
                        }
                        state = STATE_PARSING;
                    }
                }
                if (state == STATE_TITLE) titlescreen.Clock();
                if (state == STATE_MENU) menuscreen.Clock();

                ++fpscounter;
                if (fpscounter >= 25)
                {
                    Config::SetFPS(fps);
                    fpscounter = 0;
                    fps = 0;
                }
            }
        }

        SDL_FillRect(render32, nullptr, 0);
        if (state == STATE_PLAYING)
        {
            renderer.SetTeleEffect(logic.GetTeleEffect());
            renderer.SetPlayerHit(logic.GetPlayerHit());
            renderer.SetThermo(logic.GetThermo());
            renderer.Render(&cam);
            if (selectedGloomClassic) GL_PixelateWorld2x2(render32);
            MapObject player = logic.GetPlayerObj();
            RendererHooks::DeferHudRender(&hud, player, &smallfont,
                renderwidth, renderheight, logic.GetLives());
            ++fps;
        }
        else if (state == STATE_MENU)
        {
            renderer.Render(&cam);
            if (selectedGloomClassic) GL_PixelateWorld2x2(render32);
            menuscreen.Render(render32, render32, smallfont);
        }

        if (state == STATE_WAITING || state == STATE_SPLASH || state == STATE_TITLE)
        {
            SDL_BlitSurface(render8, nullptr, screen32, nullptr);
            if (state == STATE_TITLE && selectedZombieMassacre &&
                !titlescreen.WantsPlainTitleBackground() && zmTitleOverlay32)
            {
                SDL_Rect overlayDst = {0, 167, zmTitleOverlay32->w, zmTitleOverlay32->h};
                SDL_BlitSurface(zmTitleOverlay32, nullptr, screen32, &overlayDst);
            }

            if (renderwidth <= 320)
                SDL_BlitScaled(screen32, nullptr, render32, &blitrect);
            else
                GL_BlitStaticWideLikeGloom2(screen32, render32, blitrect);
        }

        if (printscreen)
        {
            const std::string filename = "ux0:/data/ZGloom/img" +
                std::to_string(screennum++) + ".bmp";
            SDL_SaveBMP(render32, filename.c_str());
            printscreen = false;
        }

        if (state != STATE_SPOOLING)
        {
            using zgloom_vita::RendererVita2D;
            static bool vitaPresenterReady = false;
            if (!vitaPresenterReady)
            {
                RendererVita2D::Get().Init(renderwidth, renderheight, 960, 544);
                RendererVita2D::Get().SetIntegerScaling(true);
                RendererVita2D::Get().SetClearColor(0xFF000000);
                RendererVita2D::Get().SetTargetFps(Config::GetMaxFps());
                vitaPresenterReady = true;
            }
            static uint32_t lastTicks = SDL_GetTicks();
            const uint32_t now = SDL_GetTicks();
            const uint32_t delta = now - lastTicks;
            lastTicks = now;
            MuzzleFlashFX::Get().ApplyToSurface(render32);
            MuzzleFlashFX::Get().Update(static_cast<float>(delta));
            RendererVita2D::Get().PresentARGB8888WithHook(
                render32->pixels, render32->pitch,
                RendererHooks::EffectsDrawOverlaysVita2D_WithHud);
        }
    }

    BGM::Shutdown();
    xmp_free_context(ctx);
    Config::Save();
    SoundHandler::Quit();
    RendererHooks::shutdown();

    SDL_FreeSurface(render8);
    SDL_FreeSurface(render32);
    SDL_FreeSurface(screen32);
    SDL_FreeSurface(intermissionscreen);
    SDL_FreeSurface(titlebitmap);
    SDL_FreeSurface(titlemenubitmap);
    SDL_FreeSurface(splashbitmap);
    if (zmTitleOverlay32) SDL_FreeSurface(zmTitleOverlay32);
    if (zmTitleOverlay8) SDL_FreeSurface(zmTitleOverlay8);
    SDL_DestroyWindow(win);
    SDL_Quit();
    sceAppUtilShutdown();
    return 0;
}
