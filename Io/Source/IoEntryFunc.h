#include "Io.h"

int AppMain(IoSurface& s);

#ifdef IO_WIN32

// Entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	IoSurface& s = IoSurfaceWin32(hInstance);
	
	return AppMain(s);
}

#endif