#include "Io/Source/Io.h"

#ifdef IO_WIN32

// Entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	IoSurface& s = IoSurfaceWin32(hInstance);

	s.Run();

	return 0;
}

#endif