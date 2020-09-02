#pragma once

#include "config.h"

#include <glm/glm.hpp>

#include <functional>
#include <mutex>
#include <condition_variable>

class IoSurface
{
public:
	virtual ~IoSurface() {};
	virtual void Run(std::function<void()> loopFunc) = 0;
	virtual void WaitForEvent() = 0;

	virtual glm::uvec2 GetSize() = 0;
};

#ifdef IO_WIN32

#include "windows.h"

class IoSurfaceWin32 : public IoSurface
{
private:
	bool Running = true;

	glm::uvec2 size = glm::uvec2(1280, 800);
	
	std::mutex m_eventLock;
	std::condition_variable m_eventCV;

public:
	HWND m_hwnd;
	HINSTANCE m_hInstance;

	IoSurfaceWin32(HINSTANCE hInstance);
	~IoSurfaceWin32();

	void Run(std::function<void()> loopFunc);
	void WaitForEvent();
	
	glm::uvec2 GetSize();

	LRESULT WindowCallbacks(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif