#pragma once

#include "Europa/Source/Europa.h"
#include "Europa/Source/EuropaUtils.h"

struct AmaltheaFrame
{
    EuropaImage::Ref image;
    EuropaImageView::Ref imageView;
    EuropaCmdlist::Ref cmdlist;
    uint32 frameIndex;
};

class AmaltheaBehaviors
{
public:
    virtual void OnDeviceCreated() = 0;
    virtual void OnDeviceDestroy() = 0;
    virtual void OnSwapChainCreated() = 0;
    virtual void OnSwapChainDestroy() = 0;
    virtual void RenderFrame(AmaltheaFrame& ctx, float time, float deltaTime) = 0;
};

class Amalthea : public AmaltheaBehaviors
{
public:
    const int MAX_FRAMES_IN_FLIGHT = 3;

protected:
    Europa& m_europa;
    REF(IoSurface) m_ioSurface;

    glm::uvec2 m_windowSize = glm::uvec2(0);

    std::vector<EuropaDevice::Ref> m_allDevices;
    EuropaDevice::Ref m_device = nullptr;

    EuropaSurface::Ref m_surface = nullptr;

    std::vector<EuropaQueueFamilyProperties> m_queueFamilies;
    EuropaQueue::Ref m_cmdQueue = nullptr;

    EuropaSwapChain::Ref m_swapChain = nullptr;
    EuropaSwapChainCapabilities m_swapChainCaps;

    EuropaCommandPool::Ref m_cmdpool;

    std::vector<EuropaSemaphore::Ref> m_imageAvailableSemaphore;
    std::vector<EuropaSemaphore::Ref> m_renderFinishedSemaphore;
    std::vector<EuropaFence::Ref> m_inFlightFences;
    std::vector<EuropaFence::Ref> m_imagesInFlight;

    std::vector<AmaltheaFrame> m_frames;

    EuropaCmdlist::Ref m_copyCmdlist;
    EuropaTransferUtil* m_transferUtil;
    EuropaStreamingBuffer* m_streamingBuffer;

private:
    void CreateDevice();
    void DestroyDevice();
    void CreateSwapChain();
    void DestroySwapChain();
    void RecreateSwapChain();

public:
    void Run();

    Amalthea(Europa& europaInstance, REF(IoSurface) ioSurface);
    virtual ~Amalthea();
};