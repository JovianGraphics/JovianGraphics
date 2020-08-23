#pragma once

#include "Europa.h"

class EuropaTransferUtil
{
private:
    EuropaDevice* m_device;
    EuropaQueue* m_queue;
    EuropaCmdlist* m_cmdlist;

    uint32 m_stagingBufferSize;
    EuropaBuffer* m_bufferCpu2Gpu;

public:

    void UploadToBufferEx(EuropaBuffer* target, uint8* src, uint32 size);
    template <typename T> void UploadToBufferEx(EuropaBuffer* target, T* src, uint32 numElements)
    {
        UploadToBufferEx(target, (uint8*)(src), uint32(sizeof(T)) * numElements);
    }

    void NewFrame();

    EuropaTransferUtil(EuropaDevice* device, EuropaQueue* queue, EuropaCmdlist* cmdlist, uint32 stagingBufferSize);
    ~EuropaTransferUtil();
};