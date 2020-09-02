#include "Io.h"

int AppMain(IoSurface::Ref s);

#ifdef IO_WIN32

// Entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	IoSurface::Ref s = std::make_shared<IoSurfaceWin32>(hInstance);
	
	return AppMain(s);
}

#endif