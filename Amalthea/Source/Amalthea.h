#pragma once

#include "Europa/Source/Europa.h"
#include "Europa/Source/EuropaUtils.h"
#include "Europa/Source/EuropaImGui.h"

#include "Ganymede/Source/GanymedeECS.h"

struct AmaltheaFrame
{
    EuropaImage::Ref image;
    EuropaImageView::Ref imageView;
    EuropaCmdlist::Ref cmdlist;
    uint32 frameIndex;
};

class Amalthea;

class AmaltheaBehaviors
{
public:
    struct Events {
        uint32 OnCreateDevice;
        uint32 OnDestroyDevice;
        uint32 OnCreateSwapChain;
        uint32 OnDestroySwapChain;
        uint32 OnRender;
    };

    typedef std::function<void(Amalthea* amalthea)> OnCreateDevice;
    typedef std::function<void(Amalthea* amalthea)> OnDestroyDevice;
    typedef std::function<void(Amalthea* amalthea)> OnCreateSwapChain;
    typedef std::function<void(Amalthea* amalthea)> OnDestroySwapChain;
    typedef std::function<void(Amalthea* amalthea, AmaltheaFrame& ctx, float time, float deltaTime)> OnRender;
};

class Amalthea
{
public:
    const int MAX_FRAMES_IN_FLIGHT = 3;

private:
    GanymedeECS& m_ecs;

public:
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
    EuropaImGui* m_imgui;

    AmaltheaBehaviors::Events m_events;

private:
    void CreateDevice();
    void DestroyDevice();
    void CreateSwapChain();
    void DestroySwapChain();
    void RecreateSwapChain();

public:
    void Run();

    Amalthea(GanymedeECS& ecs, Europa& europaInstance, REF(IoSurface) ioSurface);
    virtual ~Amalthea();
};