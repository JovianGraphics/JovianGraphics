#pragma once

#include "config.h"

#include <Ganymede/Source/Ganymede.h>

#include <glm/glm.hpp>

#include <functional>
#include <mutex>
#include <condition_variable>

GANYMEDE_ENUM(
	IoKeyboardEvent,
	(KeyDown)
	(KeyUp)
	(CharacterInput)
)

class IoSurface
{
public:
	DECL_REF(IoSurface)

	virtual ~IoSurface() {};
	virtual void Run(std::function<void()> loopFunc) = 0;
	virtual void WaitForEvent() = 0;

	virtual void SetKeyCallback(std::function<void(uint8_t, uint16, std::string, IoKeyboardEvent)> callback) = 0;
	virtual bool IsKeyDown(uint8_t key) = 0;

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

	std::function<void(uint8_t, uint16, std::string, IoKeyboardEvent)> keyCallback;

public:
	DECL_REF(IoSurfaceWin32)
		
	HWND m_hwnd;
	HINSTANCE m_hInstance;

	bool m_enableExternalCallback = false;
	std::function<LRESULT(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)> m_externalCallback;

	IoSurfaceWin32(HINSTANCE hInstance);
	~IoSurfaceWin32();

	void Run(std::function<void()> loopFunc);
	void WaitForEvent();
	
	glm::uvec2 GetSize();

	void SetKeyCallback(std::function<void(uint8_t, uint16, std::string, IoKeyboardEvent)> callback);
	bool IsKeyDown(uint8_t key);

	LRESULT WindowCallbacks(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif