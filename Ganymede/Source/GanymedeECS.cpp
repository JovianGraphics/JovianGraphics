#include "GanymedeECS.h"

uint32 GanymedeECS::RegisterEvent()
{
    if (!m_emptySlots.empty())
    {
        uint32 back = m_emptySlots.back();
        m_emptySlots.pop_back();
        return back;
    }

    m_events.push_back(std::vector<FunctionType>());
    return m_events.size() - 1;
}