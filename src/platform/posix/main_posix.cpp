// Entry point of the game on non-Windows systems: the Windows one is WinMain.
#include <windows.h>

#include "platform.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

int main(int argc, char** argv)
{
    platform::SetCommandLine(argc, argv);
    return WinMain(NULL, NULL, GetCommandLineA(), 5 /* SW_SHOW */);
}
