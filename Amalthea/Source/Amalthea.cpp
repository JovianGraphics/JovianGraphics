#include "Amalthea.h"
#include "Ganymede/Source/Ganymede.h"
#include "Io/Source/Io.h"

#include <thread>

Amalthea::Amalthea(Europa& europaInstance, IoSurface& ioSurface)
    : m_europa(europaInstance)
	, m_ioSurface(ioSurface)
{
	// Enumerate devices & select one
	m_allDevices = m_europa.GetDevices();

	for (EuropaDevice* d : m_allDevices)
	{
		GanymedePrint d->GetName(), ":", EuropaDeviceTypeToString(d->GetType());
	}

	m_device = m_allDevices[0]; // FIXME: Select the best one instead of first one

	// Create Surface and its respective Queues
	m_surface = m_europa.CreateSurface(&m_ioSurface);
	m_queueFamilies = m_device->GetQueueFamilies(m_surface);

	EuropaQueueFamilyProperties requiredQueues[2];
	bool foundGraphics = false, foundPresent = false;
	for (auto q : m_queueFamilies)
		GanymedePrint "Queue family", q.queueIndex, "has capabilities:", EuropaQueueCapabilitiesToString(q.capabilityFlags);
	uint32 i = 0;
	for (auto q : m_queueFamilies)
	{
		if (+(q.capabilityFlags & EuropaQueueCapabilitiesGraphics))
		{
			requiredQueues[0] = q;
			foundGraphics = true;
		}

		if (q.presentSupport)
		{
			requiredQueues[1] = q;
			foundPresent = true;
		}

		if (foundGraphics && foundPresent)
			break;

		i++;
	}

	if (requiredQueues[0].queueIndex == requiredQueues[1].queueIndex)
	{
		uint32_t queueCount[] = { 1 };
		m_device->CreateLogicalDevice(1, requiredQueues, queueCount);

		m_cmdQueue = m_device->GetQueue(requiredQueues[0]);
	}
	else
	{
		throw std::runtime_error("This device has a seperate graphics and present queue");
	}

	// Create Swapchain
	m_swapChainCaps = m_device->getSwapChainCapabilities(m_surface);

	GanymedePrint "Surface Formats:";
	for (EuropaSurfaceFormat& format : m_swapChainCaps.formats)
		GanymedePrint EuropaImageFormatToString(format.format), EuropaColorSpaceToString(format.colorSpace);

	EuropaSwapChainCreateInfo swapChainCreateInfo;
	swapChainCreateInfo.colorSpace = EuropaColorSpace::GammaSRGB;
	swapChainCreateInfo.extent = m_swapChainCaps.surfaceCaps.currentExtent;
	swapChainCreateInfo.format = EuropaImageFormat::BGRA8sRGB;
	swapChainCreateInfo.imageCount = 3;
	swapChainCreateInfo.numLayers = 1;
	swapChainCreateInfo.presentMode = EuropaPresentMode::Mailbox;
	swapChainCreateInfo.surface = m_surface;
	swapChainCreateInfo.surfaceTransform = m_swapChainCaps.surfaceCaps.currentTransform;

	m_swapChain = m_device->CreateSwapChain(swapChainCreateInfo);

	// Create command pool & command lists
	m_cmdpool = m_device->CreateCommandPool(requiredQueues[0]);

	// Create per-frame context
	std::vector<EuropaImage*> swapChainImages = m_device->GetSwapChainImages(m_swapChain);

	std::vector<EuropaCmdlist*> cmdlists = m_cmdpool->AllocateCommandBuffers(0, uint32(swapChainImages.size()));

	i = 0;
	for (auto image : swapChainImages)
	{
		EuropaImageViewCreateInfo info{};
		info.format = EuropaImageFormat::BGRA8sRGB;
		info.image = image;
		info.minArrayLayer = 0;
		info.numArrayLayers = 1;
		info.minMipLevel = 0;
		info.numMipLevels = 1;
		info.type = EuropaImageViewType::View2D;

		EuropaImageView* view = m_device->CreateImageView(info);

		AmaltheaFrame f;
		f.image = image;
		f.imageView = view;
		f.cmdlist = cmdlists[i];
		f.frameIndex = i;

		m_frames.push_back(f);

		i++;
	}

	// Create Synchronization primitives for Present
	m_imagesInFlight.resize(swapChainImages.size(), nullptr);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_imageAvailableSemaphore.push_back(m_device->CreateSema());
		m_renderFinishedSemaphore.push_back(m_device->CreateSema());
		m_inFlightFences.push_back(m_device->CreateFence(true));
	}

	// Create Utils
	m_copyCmdlist = m_cmdpool->AllocateCommandBuffers(0, 1)[0];
	m_transferUtil = new EuropaTransferUtil(m_device, m_cmdQueue, m_copyCmdlist, 128 << 20); // 128M
	m_streamingBuffer = new EuropaStreamingBuffer(m_device, MAX_FRAMES_IN_FLIGHT);
	m_destroyQueue = new EuropaDestroyQueue(MAX_FRAMES_IN_FLIGHT);
}

Amalthea::~Amalthea()
{
	GanymedeDelete(m_destroyQueue);
	GanymedeDelete(m_transferUtil);
	GanymedeDelete(m_streamingBuffer);
	for (auto f : m_inFlightFences) GanymedeDelete(f);
	for (auto s : m_imageAvailableSemaphore) GanymedeDelete(s);
	for (auto s : m_renderFinishedSemaphore) GanymedeDelete(s);
	GanymedeDelete(m_cmdpool);
	GanymedeDelete(m_swapChain);
	GanymedeDelete(m_surface);
	GanymedeDelete(m_cmdQueue);
}

void Amalthea::Run()
{
	this->OnDeviceCreated();

	size_t currentFrame = 0;
	size_t frames = 0;
	bool running = true;

	// Rendering thread
	std::thread rendering([&]()
	{
		auto startTime = std::chrono::steady_clock::now();
		while (running)
		{
			m_device->WaitForFences(1, &m_inFlightFences[currentFrame]);

			uint32 imageIndex = m_swapChain->AcquireNextImage(m_imageAvailableSemaphore[currentFrame]);

			if (m_imagesInFlight[imageIndex] != nullptr) m_device->WaitForFences(1, &m_imagesInFlight[imageIndex]);
			m_imagesInFlight[imageIndex] = m_inFlightFences[currentFrame];

			m_device->ResetFences(1, &m_inFlightFences[currentFrame]);

			auto currentTime = std::chrono::steady_clock::now();
			float runTime = std::chrono::duration<float>(currentTime - startTime).count();

			m_transferUtil->NewFrame();
			m_streamingBuffer->NewFrame();
			m_destroyQueue->NewFrame();
			
			AmaltheaFrame& ctx = m_frames[currentFrame];

			this->RenderFrame(ctx, runTime);

			EuropaPipelineStage waitStage = EuropaPipelineStageColorAttachmentOutput;
			m_cmdQueue->Submit(1, &m_imageAvailableSemaphore[currentFrame], &waitStage, 1, &ctx.cmdlist, 1, &m_renderFinishedSemaphore[currentFrame], m_inFlightFences[currentFrame]);
			m_cmdQueue->Present(1, &m_renderFinishedSemaphore[currentFrame], 1, &m_swapChain, imageIndex);

			currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		}
	});

	// Window thread (main thread)
	m_ioSurface.Run([]() {});

	// Terminate
	running = false;
	rendering.join();

	m_device->WaitIdle();
}
