#include "Io.h"

#ifdef IO_WIN32

#include <fcntl.h>
#include <io.h>
#include <cstdio>
#include <iostream>
#include <fstream>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    IoSurfaceWin32* s = nullptr;

    switch (uMsg)
    {
    case WM_CREATE:
    case WM_NCCREATE:
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        s = reinterpret_cast<IoSurfaceWin32*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)s);
        s->m_hwnd = hwnd;
        break;
    }

    if (!s)
    {
        LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
        s = reinterpret_cast<IoSurfaceWin32*>(ptr);
    }

    if (s)
        return s->WindowCallbacks(uMsg, wParam, lParam);
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

IoSurfaceWin32::IoSurfaceWin32(HINSTANCE hInstance)
	: m_hInstance(hInstance)
{
	const LPCSTR CLASS_NAME = "Jovian Graphics Windowed Application";

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

    m_hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        "Jovian Graphics",              // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, size.x, size.y,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        this        // Additional application data
    );

    if (m_hwnd == NULL)
    {
        // FIXME: Use project-wide standard error reporting
        throw(-1);
    }

    ShowWindow(m_hwnd, SW_SHOWDEFAULT);
}

IoSurfaceWin32::~IoSurfaceWin32()
{
    DestroyWindow(m_hwnd);
}

void IoSurfaceWin32::Run()
{
    MSG msg;
    WNDCLASS wincl;

    while (Running && GetMessage(&msg, m_hwnd, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    OnClose();
}

void IoSurfaceWin32::OnClose()
{
}

glm::uvec2 IoSurfaceWin32::GetSize()
{
    return size;
}

LRESULT IoSurfaceWin32::WindowCallbacks(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        Running = false;
        break;
    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
}

#endif