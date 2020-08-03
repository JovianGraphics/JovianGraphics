#pragma once

#include "config.h"

class IoSurface
{
public:
	virtual ~IoSurface() {};
	virtual void Run() = 0;
	virtual void OnClose() = 0;
};

#ifdef IO_WIN32

#include "windows.h"

class IoSurfaceWin32 : public IoSurface
{
private:
	bool Running = true;

public:
	HWND m_hwnd;
	HINSTANCE m_hInstance;

	IoSurfaceWin32(HINSTANCE hInstance);
	~IoSurfaceWin32();

	void Run();
	void OnClose();

	LRESULT WindowCallbacks(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif