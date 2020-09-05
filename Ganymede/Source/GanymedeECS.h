#pragma once

#include "Ganymede.h"

#include <vector>
#include <functional>

class GanymedeECS
{
private:
    typedef std::function<void()>* FunctionType;

    std::vector<std::vector<FunctionType>> m_events;
    std::vector<uint32> m_emptySlots;

public:
    uint32 RegisterEvent();

    template <typename ...Ts> void Signal(uint32 eventId, Ts... args)
    {
        std::vector<FunctionType>& handlerList = m_events[eventId];

        for (FunctionType f : handlerList)
        {
            auto derivedF = reinterpret_cast<std::function<void(Ts...)>*>(f);
            (*derivedF)(args...);
        }
    }

    template <typename T> void RegisterHandler(uint32 eventId, std::function<T>* handler)
    {
        m_events[eventId].push_back(reinterpret_cast<FunctionType>(handler));
    }
};