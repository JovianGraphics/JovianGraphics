#include "Io.h"

#ifdef IO_WIN32

#include <iostream>
#include <fstream>
#include <Windowsx.h>

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

    this->keyCallback = [](uint8, uint16, std::string, IoKeyboardEvent) {};

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

void IoSurfaceWin32::Run(std::function<void()> loopFunc)
{
    MSG msg;

    while (Running && GetMessage(&msg, m_hwnd, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        loopFunc();
    }
}

void IoSurfaceWin32::WaitForEvent()
{
    std::unique_lock<std::mutex> lk(m_eventLock);
    m_eventCV.wait(lk);
}

void IoSurfaceWin32::SetKeyCallback(std::function<void(uint8_t, uint16, std::string, IoKeyboardEvent)> callback)
{
    this->keyCallback = callback;
}

bool IoSurfaceWin32::IsKeyDown(uint8_t key)
{
    if (+(GetAsyncKeyState(key) & 0x8000))
    {
        return true;
    }
    return false;
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
    case WM_SIZE:
        this->size = glm::uvec2(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_KEYDOWN:
        this->keyCallback(uint8(wParam), uint16(wParam), "", IoKeyboardEvent::KeyDown);
        break;
    case WM_KEYUP:
        this->keyCallback(uint8(wParam), uint16(wParam), "", IoKeyboardEvent::KeyUp);
        break;
    case WM_CHAR:
        this->keyCallback(uint8(wParam), uint16(wParam), "", IoKeyboardEvent::CharacterInput);
        break;
    case WM_MOUSEMOVE:
        break;
    }

    m_eventCV.notify_all();

    if (m_enableExternalCallback)
        m_externalCallback(m_hwnd, uMsg, wParam, lParam);

    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

#endif