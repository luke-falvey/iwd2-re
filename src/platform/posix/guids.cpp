// GUID constants. Every DEFINE_GUID of the shim headers and of the DirectPlay
// headers is defined here, once, by redefining DEFINE_GUID to define. The values are the
// ones of the Windows SDK (dplay.h, dplobby.h, ddraw.h, dsound.h, unknwn.h);
// the DirectDraw and DirectSound ones written in ddraw_decl.h and
// dsound_decl.h were compared with the SDK.
#include <win32shim/wintypes.h>

// What <initguid.h> does, done before com.h is first included (com.h itself
// holds DEFINE_GUIDs).
#undef DEFINE_GUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    extern const GUID name;                                          \
    const GUID name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }

#include <win32shim/com.h>
#include <win32shim/ddraw_decl.h>
#include <win32shim/dplay_decl.h>
#include <win32shim/dsound_decl.h>
