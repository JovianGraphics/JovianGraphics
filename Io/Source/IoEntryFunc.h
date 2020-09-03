#include "Io.h"

int AppMain(IoSurface::Ref s);

#ifdef IO_WIN32

// Entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	size_t origsize = wcslen(pCmdLine);
	if (origsize > 0)
	{
		char* cmdLine = new char[origsize * 2 + 1];
		size_t convertedChars = 0;
		wcstombs_s(&convertedChars, cmdLine, origsize * 2, pCmdLine, _TRUNCATE);
		SetCurrentDirectory(cmdLine);
	}

	char curr[512];
	GetCurrentDirectory(512, curr);
	GanymedePrint "Running from", curr;

	IoSurface::Ref s = std::make_shared<IoSurfaceWin32>(hInstance);
	
	return AppMain(s);
}

#endif