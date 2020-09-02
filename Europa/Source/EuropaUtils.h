#pragma once

#include "Europa.h"

class EuropaTransferUtil
{
private:
    EuropaDevice::Ref m_device;
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

    EuropaTransferUtil(EuropaDevice::Ref device, EuropaQueue* queue, EuropaCmdlist* cmdlist, uint32 stagingBufferSize);
    ~EuropaTransferUtil();
};

class EuropaStreamingBuffer
{
public:
    struct Handle
    {
        EuropaBuffer* buffer;
        uint32 offset;

        template <typename T> T* Map() { return reinterpret_cast<T*>(buffer->Map<uint8>() + offset); };
        inline void Unmap() { buffer->Unmap(); }
    };

private:
    EuropaDevice::Ref m_device;

    std::vector<EuropaBuffer*> m_streamingBuffers;
    uint32 m_currentFrame = 0;
    uint32 m_frameCount = 0;
    uint32 m_currentFrameOffset = 0;

public:
    Handle AllocateTransient(uint32 size);
    void NewFrame();

    EuropaStreamingBuffer(EuropaDevice::Ref device, uint32 frameCount);
    ~EuropaStreamingBuffer();
};