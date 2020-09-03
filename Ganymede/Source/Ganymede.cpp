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

GanymedeScrollingBuffer::GanymedeScrollingBuffer(uint32 maxSize, uint32 offset)
    : m_maxSize(maxSize)
    , m_offset(offset)
{
    m_dataX.reserve(maxSize);
    m_dataY.reserve(maxSize);
}

void GanymedeScrollingBuffer::AddPoint(float x, float y) {
    if (m_dataX.size() < m_maxSize)
    {
        m_dataX.push_back(x);
        m_dataY.push_back(y);
    }
    else
    {
        m_dataX[m_offset] = x;
        m_dataY[m_offset] = y;
        m_offset = (m_offset + 1) % m_maxSize;
    }
}
void GanymedeScrollingBuffer::Erase() {
    if (m_dataX.size() > 0) {
        m_dataX.resize(0);
        m_dataY.resize(0);
        m_offset = 0;
    }
}