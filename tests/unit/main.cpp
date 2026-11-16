#define DOCTEST_CONFIG_IMPLEMENT
#include "test.h"

int main(int argc, char** argv)
{
#ifndef IWD2RE_NO_MFC
    // MFC needs an initialized module state for CString and friends.
    if (!AfxWinInit(GetModuleHandleA(NULL), NULL, GetCommandLineA(), 0)) {
        return 1;
    }
#endif

    doctest::Context context;
    context.applyCommandLine(argc, argv);
    return context.run();
}
