#pragma once

#include "Europa/Source/Europa.h"
#include "Europa/Source/EuropaUtils.h"

struct AmaltheaFrame
{
    EuropaImage* image;
    EuropaImageView* imageView;
    EuropaCmdlist* cmdlist;
    uint32 frameIndex;
};

class AmaltheaBehaviors
{
public:
    virtual void OnDeviceCreated() = 0;
    virtual void RenderFrame(AmaltheaFrame& ctx, float time) = 0;
};

class Amalthea : public AmaltheaBehaviors
{
public:
    const int MAX_FRAMES_IN_FLIGHT = 3;

protected:
    Europa& m_europa;
    IoSurface& m_ioSurface;

    std::vector<EuropaDevice*> m_allDevices;
    EuropaDevice* m_device = nullptr;

    EuropaSurface* m_surface = nullptr;

    std::vector<EuropaQueueFamilyProperties> m_queueFamilies;
    EuropaQueue* m_cmdQueue = nullptr;

    EuropaSwapChain* m_swapChain = nullptr;
    EuropaSwapChainCapabilities m_swapChainCaps;

    EuropaRenderPass* m_mainRenderPass = nullptr;

    EuropaCommandPool* m_cmdpool;

    std::vector<EuropaSemaphore*> m_imageAvailableSemaphore;
    std::vector<EuropaSemaphore*> m_renderFinishedSemaphore;
    std::vector<EuropaFence*> m_inFlightFences;
    std::vector<EuropaFence*> m_imagesInFlight;

    std::vector<AmaltheaFrame> m_frames;

    EuropaCmdlist* m_copyCmdlist;
    EuropaTransferUtil* m_transferUtil;
    EuropaStreamingBuffer* m_streamingBuffer;

public:
    void Run();

    Amalthea(Europa& europaInstance, IoSurface& ioSurface);
    ~Amalthea();
};