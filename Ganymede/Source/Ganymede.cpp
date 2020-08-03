#include "Ganymede.h"

#ifdef IO_WIN32
#include <windows.h>
#else
#include <iostream>
#endif

#include <cstdarg>

void GanymedeOutputDebugString(const char* str)
{
#ifdef IO_WIN32
	OutputDebugString(str);
#else
	std::cout << str;
#endif
}