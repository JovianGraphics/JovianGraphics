#include "EuropaUtils.h"

void EuropaTransferUtil::UploadToBufferEx(EuropaBuffer* target, uint8* src, uint32 size)
{
    uint32 offset = 0;

    while (offset < size)
    {
        uint32 chunkSize = size - offset > m_stagingBufferSize ? m_stagingBufferSize : size - offset;

        uint8* mappedBuffer = m_bufferCpu2Gpu->Map<uint8>();
        memcpy(mappedBuffer, src + offset, size);
        m_bufferCpu2Gpu->Unmap();

        m_cmdlist->Begin();
        m_cmdlist->CopyBuffer(target, m_bufferCpu2Gpu, chunkSize, 0, offset);
        m_cmdlist->End();

        m_queue->Submit(m_cmdlist);
        m_queue->WaitIdle();

        offset += m_stagingBufferSize;
    }
}

void EuropaTransferUtil::NewFrame()
{
}

EuropaTransferUtil::EuropaTransferUtil(EuropaDevice* device, EuropaQueue* queue, EuropaCmdlist* cmdlist, uint32 stagingBufferSize)
    : m_device(device)
    , m_queue(queue)
    , m_cmdlist(cmdlist)
    , m_stagingBufferSize(stagingBufferSize)
{
    EuropaBufferInfo bufferInfo;
    bufferInfo.exclusive = true;
    bufferInfo.memoryUsage = EuropaMemoryUsage::Cpu2Gpu;
    bufferInfo.size = stagingBufferSize;
    bufferInfo.usage = EuropaBufferUsage::EuropaBufferUsageTransferSrc;

    m_bufferCpu2Gpu = m_device->CreateBuffer(bufferInfo);
}

EuropaTransferUtil::~EuropaTransferUtil()
{
    GanymedeDelete(m_bufferCpu2Gpu);
}

EuropaStreamingBuffer::Handle EuropaStreamingBuffer::AllocateTransient(uint32 size)
{
    Handle h;
    h.buffer = m_streamingBuffers[m_currentFrame];
    h.offset = m_currentFrameOffset;

    m_currentFrameOffset += size;

    return h;
}

void EuropaStreamingBuffer::NewFrame()
{
    m_currentFrame = (m_currentFrame + 1) % m_frameCount;
    m_currentFrameOffset = 0;
}

EuropaStreamingBuffer::EuropaStreamingBuffer(EuropaDevice* device, uint32 frameCount)
    : m_device(device)
    , m_frameCount(frameCount)
{
    EuropaBufferInfo bufferInfo;
    bufferInfo.exclusive = true;
    bufferInfo.memoryUsage = EuropaMemoryUsage::Cpu2Gpu;
    bufferInfo.size = 16 << 20;
    bufferInfo.usage = EuropaBufferUsage::EuropaBufferUsageTransferSrc;

    for (uint32 i = 0; i < frameCount; i++)
    {
        m_streamingBuffers.push_back(m_device->CreateBuffer(bufferInfo));
    }
}

EuropaStreamingBuffer::~EuropaStreamingBuffer()
{
    for (auto b : m_streamingBuffers) GanymedeDelete(b);
}
