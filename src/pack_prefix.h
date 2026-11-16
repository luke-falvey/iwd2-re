#ifndef PACK_PREFIX_H_
#define PACK_PREFIX_H_

// Force-included before every source file of the game.
//
// The original was built with 2-byte struct packing (/Zp2). Current Windows
// SDK headers refuse to compile that way, so every system header the project
// uses is included here with the default packing, and only then is packing
// switched to 2 for the project's own headers. A system header that is first
// included after this point would be packed wrongly: add it here instead.

#ifdef IWD2_INITGUID
#include <initguid.h>
#endif

#include <winsock2.h>

// commdlg.h (pulled in above) defines GetFileTitle as a macro, which would
// rename CFile::GetFileTitle in the MFC headers below.
#undef GetFileTitle

#ifdef IWD2RE_NO_MFC
// The replacement classes set their own packing, like the MFC headers do.
#include <windows.h>

#include "compat/afxcompat.h"
#else
#include <afx.h>
#include <afxmt.h>
#include <afxtempl.h>
#include <afxwin.h>

#include <windows.h>
#endif

#include <dbghelp.h>
#include <ddraw.h>
#include <dplay.h>
#include <dplobby.h>
#include <dsound.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <mmsystem.h>
#include <tlhelp32.h>
#include <winver.h>

#include <direct.h>
#include <io.h>
#include <limits.h>
#include <math.h>
#include <process.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bitset>
#include <cmath>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <zlib.h>

#pragma pack(2)

#endif /* PACK_PREFIX_H_ */
