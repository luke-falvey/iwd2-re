#include "test.h"

#include <new>

#include "CChitin.h"
#include "CVidImage.h"
#include "CVidInf.h"
#include "CVidPalette.h"
#include "CWarp.h"

namespace {

// Just enough of the engine for `CVidPalette::RealizeRange`: it reads the
// current video mode through `g_pChitin->pActiveEngine->pVidMode`, the 3D flag
// and the AU counter. The real `CChitin` and `CWarp` constructors need a
// window and game data, so the test uses zeroed storage for them.
struct FakeEngine {
    FakeEngine()
    {
        memset(chitinStorage, 0, sizeof(chitinStorage));
        memset(warpStorage, 0, sizeof(warpStorage));

        pChitin = reinterpret_cast<CChitin*>(chitinStorage);
        CWarp* pWarp = reinterpret_cast<CWarp*>(warpStorage);

        pVidMode = new CVidInf();
        pVidMode->m_dwRBitShift = 16;
        pVidMode->m_dwGBitShift = 8;
        pVidMode->m_dwBBitShift = 0;

        pWarp->pVidMode = pVidMode;
        pChitin->pActiveEngine = pWarp;
        pChitin->cVideo.m_bIs3dAccelerated = FALSE;
        pChitin->nAUCounter = 0;

        pSavedChitin = g_pChitin;
        g_pChitin = pChitin;
    }

    ~FakeEngine()
    {
        g_pChitin = pSavedChitin;
        delete pVidMode;
    }

    alignas(8) unsigned char chitinStorage[sizeof(CChitin)];
    alignas(8) unsigned char warpStorage[sizeof(CWarp)];
    CChitin* pChitin;
    CChitin* pSavedChitin;
    CVidInf* pVidMode;
};

// Same defaults as `CVidImage::CVidImage`.
CVIDIMG_PALETTEAFFECT DefaultAffects()
{
    CVIDIMG_PALETTEAFFECT affects;
    memset(&affects, 0, sizeof(affects));
    affects.rgbTintColor = RGB(255, 255, 255);
    affects.rgbAddColor = RGB(0, 0, 0);
    affects.rgbLightColor = RGB(8, 8, 8);
    return affects;
}

void FillRanges(CVidPalette& palette)
{
    for (int nIndex = 4; nIndex < 88; nIndex++) {
        palette.m_pPalette[nIndex].rgbRed = 0xA0;
        palette.m_pPalette[nIndex].rgbGreen = 0x80;
        palette.m_pPalette[nIndex].rgbBlue = 0x40;
        palette.m_pPalette[nIndex].rgbReserved = 0;
    }
}

} // namespace

TEST_CASE("CVidPalette::RealizeRange keeps range colours with a white tint at 32 bpp")
{
    FakeEngine engine;

    // `CSize`-independent setup: a range palette like a character's.
    CVidPalette palette(CVidPalette::TYPE_RANGE);
    FillRanges(palette);

    CVIDIMG_PALETTEAFFECT affects = DefaultAffects();
    DWORD dest[256];
    memset(dest, 0, sizeof(dest));

    // Flags seen on party animations: global tint (0x10000) and the
    // per-image tint (0x20000), plus the shadow and mirror bits.
    palette.Realize(dest, 32, 0x30094, &affects, 255);

    // White global tint times white image tint is 255 * 255 >> 16, so each
    // channel loses at most two steps.
    CHECK(dest[20] == (0xFF000000 | (0x9E << 16) | (0x7F << 8) | 0x3F));
    CHECK(dest[100] == (0xFF000000 | (0x9E << 16) | (0x7F << 8) | 0x3F));
    CHECK(dest[200] == (0xFF000000 | (0x9E << 16) | (0x7F << 8) | 0x3F));
}

TEST_CASE("CVidPalette::RealizeRange applies a pulsing range light without wrapping")
{
    FakeEngine engine;

    CVidPalette palette(CVidPalette::TYPE_RANGE);
    FillRanges(palette);

    CVIDIMG_PALETTEAFFECT affects = DefaultAffects();
    COLORREF rgbLight = RGB(255, 255, 255);
    affects.pRangeLights[0] = &rgbLight;
    affects.aRangeLightPeriods[0] = 2;

    // `m_nAUCounterBase` replaces the palette address in the pulse phase.
    // With a period of 2 and counter 2 + (1 << 0) = 3, the pulse is at
    // 3 % 4 = 3 -> 2 * 2 - 3 = 1, half way: light = 1 * 255 / 2 + 8 = 135.
    palette.m_nAUCounterBase = 2;

    DWORD dest[256];
    memset(dest, 0, sizeof(dest));
    palette.Realize(dest, 32, 0, &affects, 255);

    // 0x40 * 135 / 8 = 1080 -> clamped to 255; same for the other channels.
    CHECK(dest[4] == (0xFF000000 | (0xFF << 16) | (0xFF << 8) | 0xFF));

    // Full pulse: counter 1 + 1 = 2 -> 2 % 4 = 2, light = 2 * 255 / 2 + 8 =
    // 263. The original keeps it in an int; a byte would wrap it to 7 and
    // darken the range instead.
    palette.m_nAUCounterBase = 1;
    palette.Realize(dest, 32, 0, &affects, 255);
    CHECK(dest[4] == (0xFF000000 | (0xFF << 16) | (0xFF << 8) | 0xFF));

    // Range 1 has no light and stays as it was.
    CHECK(dest[16] == (0xFF000000 | (0xA0 << 16) | (0x80 << 8) | 0x40));
}
