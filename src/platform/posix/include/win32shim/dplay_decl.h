#ifndef WIN32SHIM_DPLAY_DECL_H_
#define WIN32SHIM_DPLAY_DECL_H_
#include <win32shim/wintypes.h>
#include <win32shim/com.h>

// DirectPlay: the project's own copy of the (DirectX 6 era) dplay.h and
// dplobby.h in third_party/directplay is the declaration source. It only needs
// the COM macros of com.h. That copy defines `DWORD_PTR` as a macro unless
// `_WIN64` is set; the typedef of wintypes.h is the right one here.
#define _WIN64 1
#include "../../../../../third_party/directplay/include/dplay.h"
#include "../../../../../third_party/directplay/include/dplobby.h"
#undef _WIN64

#endif /* WIN32SHIM_DPLAY_DECL_H_ */
