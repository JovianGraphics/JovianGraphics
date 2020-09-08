#include "Amalthea.h"
#include "Ganymede/Source/Ganymede.h"
#include "Io/Source/Io.h"

#include <thread>

Amalthea::Amalthea(GanymedeECS& ecs, Europa& europaInstance, IoSurface::Ref ioSurface)
    : m_ecs(ecs)
	, m_europa(europaInstance)
	, m_ioSurface(ioSurface)
{
	m_events.OnCreateDevice = m_ecs.RegisterEvent();
	m_events.OnCreateSwapChain = m_ecs.RegisterEvent();
	m_events.OnDestroyDevice = m_ecs.RegisterEvent();
	m_events.OnDestroySwapChain = m_ecs.RegisterEvent();
	m_events.OnRender = m_ecs.RegisterEvent();
}

Amalthea::~Amalthea()
{
}

void Amalthea::CreateDevice()
{
	// Enumerate devices & select one
	m_allDevices = m_europa.GetDevices();

	for (EuropaDevice::Ref d : m_allDevices)
	{
		GanymedePrint d->GetName(), ":", EuropaDeviceTypeToString(d->GetType());
	}

	m_device = m_allDevices[0]; // FIXME: Select the best one instead of first one

	// Create Surface and its respective Queues
	m_surface = m_europa.CreateSurface(m_ioSurface);
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
		std::vector<uint32> queueCount;
		queueCount.resize(m_queueFamilies.size(), 1);
		m_device->CreateLogicalDevice(m_queueFamilies.size(), m_queueFamilies.data(), queueCount.data());

		m_selectedQueueFamily = requiredQueues[0];
		m_cmdQueue = m_device->GetQueue(requiredQueues[0]);
	}
	else
	{
		throw std::runtime_error("This device has a seperate graphics and present queue");
	}

	// Create command pool & command lists
	m_cmdpool = m_device->CreateCommandPool(m_cmdQueue);

	// Create Utils
	m_copyCmdlist = m_cmdpool->AllocateCommandBuffers(0, 1)[0];
	m_transferUtil = new EuropaTransferUtil(m_device, m_cmdQueue, m_copyCmdlist, 128 << 20); // 128M
	m_streamingBuffer = new EuropaStreamingBuffer(m_device, MAX_FRAMES_IN_FLIGHT);
	m_imgui = new EuropaImGui(m_europa, EuropaImageFormat::BGRA8sRGB, m_device, m_cmdQueue, m_surface, m_ioSurface);

	// Launch Signal
	m_ecs.Signal(m_events.OnCreateDevice, this);
}

void Amalthea::DestroyDevice()
{
	m_ecs.Signal(m_events.OnDestroyDevice, this);

	GanymedeDelete(m_imgui);
	GanymedeDelete(m_transferUtil);
	GanymedeDelete(m_streamingBuffer);
}

void Amalthea::CreateSwapChain()
{
	m_windowSize = m_ioSurface->GetSize();

	// Create Swapchain
	m_swapChainCaps = m_device->getSwapChainCapabilities(m_surface);

	GanymedePrint "Surface Formats:";
	for (EuropaSurfaceFormat& format : m_swapChainCaps.formats)
		GanymedePrint EuropaImageFormatToString(format.format), EuropaColorSpaceToString(format.colorSpace);

	EuropaSwapChainCreateInfo swapChainCreateInfo;
	swapChainCreateInfo.colorSpace = EuropaColorSpace::GammaSRGB;
	swapChainCreateInfo.extent = m_windowSize;
	swapChainCreateInfo.format = EuropaImageFormat::BGRA8sRGB;
	swapChainCreateInfo.imageCount = 3;
	swapChainCreateInfo.numLayers = 1;
	swapChainCreateInfo.presentMode = EuropaPresentMode::FIFORelaxed;
	swapChainCreateInfo.surface = m_surface;
	swapChainCreateInfo.surfaceTransform = m_swapChainCaps.surfaceCaps.currentTransform;

	m_swapChain = m_device->CreateSwapChain(swapChainCreateInfo);

	// Create per-frame context
	std::vector<EuropaImage::Ref> swapChainImages = m_device->GetSwapChainImages(m_swapChain);

	std::vector<EuropaCmdlist::Ref> cmdlists = m_cmdpool->AllocateCommandBuffers(0, uint32(swapChainImages.size()));

	uint32 i = 0;
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

		EuropaImageView::Ref view = m_device->CreateImageView(info);

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

	// Utils
	std::vector<EuropaImageView::Ref> views;
	for (auto f : m_frames) views.push_back(f.imageView);
	m_imgui->RebuildSwapChain(m_frames.size(), m_windowSize, views.data(), m_copyCmdlist);

	m_ecs.Signal(m_events.OnCreateSwapChain, this);
}

void Amalthea::DestroySwapChain()
{
	m_ecs.Signal(m_events.OnDestroySwapChain, this);

	m_inFlightFences.clear();
	m_imagesInFlight.clear();
	m_imageAvailableSemaphore.clear();
	m_renderFinishedSemaphore.clear();
	m_frames.clear();

	m_swapChain = nullptr;
}

void Amalthea::RecreateSwapChain()
{
	auto caps = m_device->getSwapChainCapabilities(m_surface);
	while (caps.surfaceCaps.maxExtent == glm::uvec2(0))
	{
		m_windowSize = caps.surfaceCaps.maxExtent;
		m_ioSurface->WaitForEvent();

		caps = m_device->getSwapChainCapabilities(m_surface);
	}

	m_device->WaitIdle();

	DestroySwapChain();
	CreateSwapChain();
}

void Amalthea::Run()
{
	CreateDevice();
	CreateSwapChain();

	size_t currentFrame = 0;
	size_t frames = 0;
	bool running = true;

	// Rendering thread
	std::thread rendering([&]()
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		auto lastTime = startTime;
		while (running)
		{
			m_device->WaitForFences(1, &m_inFlightFences[currentFrame]);

			bool resized = m_windowSize != m_ioSurface->GetSize();

			int32 imageIndex = m_swapChain->AcquireNextImage(m_imageAvailableSemaphore[currentFrame]);

			if (resized || imageIndex == EuropaSwapChain::NextImageOutOfDate || imageIndex == EuropaSwapChain::NextImageSubOptimal)
			{
				RecreateSwapChain();
				currentFrame = 0;
				continue;
			}

			if (m_imagesInFlight[imageIndex] != nullptr) m_device->WaitForFences(1, &m_imagesInFlight[imageIndex]);
			m_imagesInFlight[imageIndex] = m_inFlightFences[currentFrame];

			m_device->ResetFences(1, &m_inFlightFences[currentFrame]);

			auto currentTime = std::chrono::high_resolution_clock::now();
			float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
			float runTime = std::chrono::duration<float>(currentTime - startTime).count();
			lastTime = currentTime;

			m_transferUtil->NewFrame();
			m_streamingBuffer->NewFrame();
			m_imgui->NewFrame();

			AmaltheaFrame& ctx = m_frames[currentFrame];

			ctx.cmdlist->Begin();
			
			m_ecs.Signal(m_events.OnRender, this, ctx, runTime, deltaTime);

			ImGui::Render();
			ctx.cmdlist->Barrier(ctx.image, EuropaAccessNone, EuropaAccessNone, EuropaImageLayout::Present, EuropaImageLayout::ColorAttachment, EuropaPipelineStageFragmentShader, EuropaPipelineStageFragmentShader);
			m_imgui->Render(currentFrame, ctx.cmdlist);

			ctx.cmdlist->End();

			EuropaPipelineStage waitStage = EuropaPipelineStageColorAttachmentOutput;
			m_cmdQueue->Submit(1, &m_imageAvailableSemaphore[currentFrame], &waitStage, 1, &ctx.cmdlist, 1, &m_renderFinishedSemaphore[currentFrame], m_inFlightFences[currentFrame]);
			m_cmdQueue->Present(1, &m_renderFinishedSemaphore[currentFrame], 1, &m_swapChain, imageIndex);

			currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		}
	});

	// Window thread (main thread)
	m_ioSurface->Run([]() {});

	// Terminate
	running = false;
	rendering.join();

	m_device->WaitIdle();

	DestroySwapChain();
	DestroyDevice();
}
