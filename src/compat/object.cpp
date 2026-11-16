#include "compat/afxcompat.h"

void AfxThrowInvalidArgException()
{
    // One object for all throws, never deleted, as in MFC.
    static CInvalidArgException exception(FALSE);
    throw &exception;
}

void AfxThrowInvalidStringArgException()
{
    throw new CInvalidArgException();
}
