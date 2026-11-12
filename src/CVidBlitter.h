#ifndef CVIDBLITTER_H_
#define CVIDBLITTER_H_

#include "mfc.h"

#include <ddraw.h>

class CVidBlitter {
public:
    typedef HRESULT (CVidBlitter::*BltFunc)(IDirectDrawSurface*, LPRECT, IDirectDrawSurface*, LPRECT, DWORD, LPDDBLTFX);

    CVidBlitter();
    HRESULT DxBlt(IDirectDrawSurface* lpDDSurface, LPRECT lpDestRect, IDirectDrawSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx);
    HRESULT DxBltFast(IDirectDrawSurface* lpDDSurface, LPRECT lpDestRect, IDirectDrawSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx);
    HRESULT Blt(IDirectDrawSurface* lpDDSurface, LPRECT lpDestRect, IDirectDrawSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx);
    HRESULT BltFast(IDirectDrawSurface* lpDDSurface, DWORD dwX, DWORD dwY, IDirectDrawSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans);
    void Init();
    void Reset();
    HRESULT SoftBlt(IDirectDrawSurface* lpDDSurface, LPRECT lpDestRect, IDirectDrawSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx);
    HRESULT SoftSrcKeyBlt(IDirectDrawSurface* lpDDSurface, LPRECT lpDestRect, IDirectDrawSurface* lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx);
    HRESULT FUN_007b6b20(IDirectDrawSurface* lpDDSurface, LPRECT lpDestRect, IDirectDrawSurface* lpDDSrcSurface, LPRECT lpSrcRect);
    void FUN_007b6cc0(BYTE* pDest, BYTE* pSrc, int nBytes);
    void FUN_007b6dc0(BYTE* pDest, BYTE* pSrc, int nBytes, DWORD dwColorKey);
    void FUN_007b6f20(BYTE* pDest, BYTE* pSrc, int nBytes, DWORD dwColorKey);
    HRESULT FUN_007b7070(IDirectDrawSurface* lpDDSurface, LPRECT lpDestRect, LPDDBLTFX lpDDBltFx);

    /* 0000 */ BOOLEAN m_bSoftSrcKeyBltFast; // #guess
    /* 0001 */ BOOLEAN m_bSoftBltFast; // #guess
    /* 0002 */ BOOLEAN m_bSoftSrcKeyBlt; // #guess
    /* 0003 */ BOOLEAN m_bSoftBlt; // #guess
    /* 0004 */ BOOLEAN m_bSoftMirrorBlt; // #guess
    /* 0006 */ DDSURFACEDESC m_ddsdDest; // #guess
    /* 0072 */ DDSURFACEDESC m_ddsdSrc; // #guess
    /* 00DE */ BYTE* m_pSrcSurface;
    /* 00E2 */ BYTE* m_pDestSurface;
    /* 00E6 */ unsigned char field_E6;
    /* 00E8 */ BltFunc m_pfnBltNoFX; // #guess
    /* 00EC */ BltFunc m_pfnSrcKeyBltNoFX; // #guess
    /* 00F0 */ BltFunc m_pfnBltFX; // #guess
    /* 00F4 */ BltFunc m_pfnSrcKeyBltFX; // #guess
    /* 00F8 */ BltFunc m_pfnBltFast; // #guess
    /* 00FC */ BltFunc m_pfnSrcKeyBltFast; // #guess
};

#endif /* CVIDBLITTER_H_ */
