#include "CSwitchCDStatus.h"

#include "CBaldurChitin.h"
#include "CVideo.h"

// NOTE: Layout values for the "insert CD" dialog. Only the ones touched by
// `Init` are named here.

// 0x8AE4A0
static int dword_8AE4A0 = 280;

// 0x8AE4A4
static int dword_8AE4A4 = 228;

// 0x8AE4A8
static int dword_8AE4A8 = 276;

// 0x8AE4AC
static int dword_8AE4AC = 291;

// 0x8AE4B0
static int dword_8AE4B0 = 306;

// 0x8AE4B4
static int dword_8AE4B4 = 321;

// 0x8AE4B8
static int dword_8AE4B8 = 10;

// 0x8E0320
static int dword_8E0320;

// 0x8E0324
static int dword_8E0324;

// 0x8E0330
static int dword_8E0330;

// 0x8E0334
static int dword_8E0334;

// 0x54D280
CSwitchCDStatus::CSwitchCDStatus()
{
    field_0 = 0;
    field_4 = 0;
    m_bActivateEngine = FALSE;
}

// 0x54D360
CSwitchCDStatus::~CSwitchCDStatus()
{
    if (field_4 != 0) {
        field_2D8.pRes->Release();
        field_440.pRes->Release();
        field_4F4.pRes->Release();
        field_9F4.pRes->Release();
        field_4 = 0;
    }
}

// 0x54D460
void CSwitchCDStatus::Init()
{
    LONG x;
    LONG y;

    if (g_pBaldurChitin->field_4A28 != 0) {
        dword_8AE4A0 *= 2;
        dword_8AE4AC *= 2;
        dword_8AE4A8 *= 2;
        dword_8AE4A4 *= 2;
        dword_8AE4B8 *= 2;
        dword_8AE4B4 *= 2;
        dword_8AE4B0 *= 2;
        dword_8E0330 *= 2;
        dword_8E0324 *= 2;
        dword_8E0320 *= 2;
        dword_8E0334 *= 2;

        x = CVideo::SCREENWIDTH / 2 - CBaldurChitin::DEFAULT_SCREEN_WIDTH;
        y = CVideo::SCREENHEIGHT / 2 - CBaldurChitin::DEFAULT_SCREEN_HEIGHT;
    } else {
        x = (CVideo::SCREENWIDTH - CBaldurChitin::DEFAULT_SCREEN_WIDTH) / 2;
        y = (CVideo::SCREENHEIGHT - CBaldurChitin::DEFAULT_SCREEN_HEIGHT) / 2;
    }

    dword_8AE4A0 += x;
    dword_8AE4A4 += y;
    dword_8AE4A8 += y;
    dword_8AE4AC += y;
    dword_8AE4B0 += y;
    dword_8AE4B4 += y;
    dword_8E0320 += x;
    dword_8E0324 += y;
}

// 0x54D5A0
void CSwitchCDStatus::Update(BOOLEAN a1, BOOLEAN a2, BYTE a3, const CString& a4, BOOLEAN a5, BOOLEAN a6, BOOLEAN a7)
{
    // TODO: Incomplete.
}
