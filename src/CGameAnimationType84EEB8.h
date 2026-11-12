#ifndef CGAMEANIMATIONTYPE84EEB8_H_
#define CGAMEANIMATIONTYPE84EEB8_H_

#include <string>

#include "CGameAnimationType.h"

class CProjectileAreaVisual2;
class CProjectileAreaVisual3;

// NOTE: The class name is not known. Animation of the pieces created by
// `CProjectileArea84F0DC::AIUpdate`. Vtable 0x84EEB8, size 0x606. Only the
// constructors are declared; the layout beyond `CGameAnimationType` is not
// scaffolded yet.
class CGameAnimationType84EEB8 : public CGameAnimationType {
public:
    CGameAnimationType84EEB8(const CProjectileAreaVisual3* a1, BYTE nDirection);
    CGameAnimationType84EEB8(const CProjectileAreaVisual2* a1, BYTE nDirection);

    // NOTE: Sets the BAM to play from its resource name.
    void sub_55DBD0(const char* pName);

    /* 03FE */ BYTE field_3FE[0x1F8];
    // NOTE: Original layout uses MSVC 6 `std::string` (16 bytes).
    /* 05F6 */ std::string field_5F6;
};

#endif /* CGAMEANIMATIONTYPE84EEB8_H_ */
