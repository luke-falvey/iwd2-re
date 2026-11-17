#ifndef PORT_POINTER_H_
#define PORT_POINTER_H_

// NOTE: Port: not in the original. While an area or a game is being saved,
// the original parks the address of a temporary array in a 32-bit field of
// a file structure (a point or item "start index", a creature "offset") and
// the caller swaps it for the real index a moment later. A pointer does not
// fit in such a field in a 64-bit build, so there the field holds a ticket
// for the pointer instead. In the 32-bit build these are the original casts.

#include <stdint.h>

#if INTPTR_MAX == INT32_MAX

inline DWORD PortStashPointer(void* p)
{
    return reinterpret_cast<DWORD>(p);
}

inline void* PortPeekPointer(DWORD ticket)
{
    return reinterpret_cast<void*>(ticket);
}

inline void* PortTakePointer(DWORD ticket)
{
    return reinterpret_cast<void*>(ticket);
}

#else

// Implemented in PortPointer.cpp.
DWORD PortStashPointer(void* p);
void* PortPeekPointer(DWORD ticket);
void* PortTakePointer(DWORD ticket);

#endif

#endif /* PORT_POINTER_H_ */
