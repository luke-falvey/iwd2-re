#include "PortPointer.h"

#if INTPTR_MAX != INT32_MAX

#include <map>
#include <mutex>

// NOTE: Port: see PortPointer.h. Tickets start high so that a ticket is
// never mistaken for a small index, and are never 0.
static std::mutex* PortPointerMutex()
{
    static std::mutex* mutex = new std::mutex();
    return mutex;
}

static std::map<DWORD, void*>* PortPointerMap()
{
    static std::map<DWORD, void*>* map = new std::map<DWORD, void*>();
    return map;
}

static DWORD s_nextTicket = 0x80000000;

DWORD PortStashPointer(void* p)
{
    if (p == NULL) {
        return 0;
    }

    std::lock_guard<std::mutex> lock(*PortPointerMutex());
    DWORD ticket = s_nextTicket++;
    if (s_nextTicket == 0) {
        s_nextTicket = 0x80000000;
    }
    (*PortPointerMap())[ticket] = p;
    return ticket;
}

void* PortPeekPointer(DWORD ticket)
{
    std::lock_guard<std::mutex> lock(*PortPointerMutex());
    std::map<DWORD, void*>::iterator it = PortPointerMap()->find(ticket);
    return it != PortPointerMap()->end() ? it->second : NULL;
}

void* PortTakePointer(DWORD ticket)
{
    std::lock_guard<std::mutex> lock(*PortPointerMutex());
    std::map<DWORD, void*>::iterator it = PortPointerMap()->find(ticket);
    if (it == PortPointerMap()->end()) {
        return NULL;
    }
    void* p = it->second;
    PortPointerMap()->erase(it);
    return p;
}

#endif
