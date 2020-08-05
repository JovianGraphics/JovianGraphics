#include "Io/Source/Io.h"
#include "Europa/Source/Europa.h"
#include "Europa/Source/EuropaVk.h"
#include "Ganymede/Source/Ganymede.h"

#ifdef IO_WIN32

// Entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	IoSurface& s = IoSurfaceWin32(hInstance);
	Europa& europa = EuropaVk();

	std::vector<EuropaDevice*> devices = europa.GetDevices();
	for (EuropaDevice* d : devices)
	{
		GanymedePrint d->GetName(), ":", EuropaDeviceTypeNames[uint32_t(d->GetType())];
	}

	EuropaDevice* selectedDevice = devices[1]; // Integrated GPU for this AMD Ryzen laptop

	EuropaSurface* surface = europa.CreateSurface(&s);

	std::vector<EuropaQueueFamilyProperties> queueFamily = selectedDevice->GetQueueFamilies(surface);

	EuropaQueueFamilyProperties requiredQueues[2];
	bool foundGraphics = false, foundPresent = false;
	for (auto q : queueFamily)
	{
		if (+(q.capabilityFlags & EuropaQueueCapabilities::Graphics))
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
	}
	uint32_t queueCount[] = { 1 };

	EuropaQueue* cmdQueue = nullptr;

	if (requiredQueues[0].queueIndex == requiredQueues[1].queueIndex)
	{
		selectedDevice->CreateLogicalDevice(1, requiredQueues, queueCount);

		EuropaQueue* cmdQueue = selectedDevice->GetQueue(requiredQueues[0]);
	}
	else
	{
		throw std::runtime_error("This device has a seperate graphics and present queue");
	}

	EuropaSwapChainCapabilities swapChainCaps = selectedDevice->getSwapChainCapabilities(surface);

	EuropaSwapChainCreateInfo swapChainCreateInfo;
	swapChainCreateInfo.colorSpace = EuropaColorSpace::GammaSRGB;
	swapChainCreateInfo.extent = swapChainCaps.surfaceCaps.currentExtent;
	swapChainCreateInfo.format = EuropaImageFormat::BGRA8sRGB;
	swapChainCreateInfo.imageCount = 3;
	swapChainCreateInfo.numLayers = 1;
	swapChainCreateInfo.presentMode = EuropaPresentMode::FIFORelaxed;
	swapChainCreateInfo.surface = surface;
	swapChainCreateInfo.surfaceTransform = swapChainCaps.surfaceCaps.currentTransform;

	EuropaSwapChain* swapChain = selectedDevice->CreateSwapChain(swapChainCreateInfo);

	s.Run();

	GanymedeDelete(surface);
	GanymedeDelete(cmdQueue);

	return 0;
}

#endif