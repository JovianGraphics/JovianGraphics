#include "Io/Source/Io.h"
#include "Io/Source/IoEntryFunc.h"
#include "Europa/Source/Europa.h"
#include "Europa/Source/EuropaVk.h"
#include "Ganymede/Source/Ganymede.h"

#include "triangle.frag.h"
#include "triangle.vert.h"

#include <thread>

int AppMain(IoSurface& s)
{
	Europa& europa = EuropaVk();

	std::vector<EuropaDevice*> devices = europa.GetDevices();
	for (EuropaDevice* d : devices)
	{
		GanymedePrint d->GetName(), ":", EuropaDeviceTypeToString(d->GetType());
	}

	EuropaDevice* selectedDevice = devices[0];

	EuropaSurface* surface = europa.CreateSurface(&s);

	std::vector<EuropaQueueFamilyProperties> queueFamily = selectedDevice->GetQueueFamilies(surface);

	EuropaQueueFamilyProperties requiredQueues[2];
	bool foundGraphics = false, foundPresent = false;
	for (auto q : queueFamily)
		GanymedePrint "Queue family", q.queueIndex, "has capabilities:" , EuropaQueueCapabilitiesToString(q.capabilityFlags);
	for (auto q : queueFamily)
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
	}
	uint32_t queueCount[] = { 1 };

	EuropaQueue* cmdQueue = nullptr;

	if (requiredQueues[0].queueIndex == requiredQueues[1].queueIndex)
	{
		selectedDevice->CreateLogicalDevice(1, requiredQueues, queueCount);

		cmdQueue = selectedDevice->GetQueue(requiredQueues[0]);
	}
	else
	{
		throw std::runtime_error("This device has a seperate graphics and present queue");
	}

	EuropaSwapChainCapabilities swapChainCaps = selectedDevice->getSwapChainCapabilities(surface);

	GanymedePrint "Surface Formats:";
	for (EuropaSurfaceFormat& format : swapChainCaps.formats)
		GanymedePrint EuropaImageFormatToString(format.format), EuropaColorSpaceToString(format.colorSpace);

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

	std::vector<EuropaImage*> swapChainImages = selectedDevice->GetSwapChainImages(swapChain);
	std::vector<EuropaImageView*> swapChainImageViews;

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

		swapChainImageViews.push_back(selectedDevice->CreateImageView(info));
	}

	EuropaShaderModule* shaderFragment = selectedDevice->CreateShaderModule(shader_spv_triangle_frag, sizeof(shader_spv_triangle_frag));
	EuropaShaderModule* shaderVertex = selectedDevice->CreateShaderModule(shader_spv_triangle_vert, sizeof(shader_spv_triangle_vert));

	EuropaPipelineLayout* pipelineLayout = selectedDevice->CreatePipelineLayout(EuropaPipelineLayoutInfo{ 0, 0 });

	EuropaRenderPass* renderpass = selectedDevice->CreateRenderPassBuilder();
	renderpass->AddAttachment(EuropaAttachmentInfo{
		EuropaImageFormat::BGRA8sRGB,
		EuropaAttachmentLoadOp::Clear,
		EuropaAttachmentStoreOp::Store,
		EuropaAttachmentLoadOp::DontCare,
		EuropaAttachmentStoreOp::DontCare,
		EuropaImageLayout::Undefined,
		EuropaImageLayout::Present
	});
	std::vector<EuropaAttachmentReference> attachments = { { 0, EuropaImageLayout::ColorAttachment } };
	renderpass->AddSubpass(EuropaPipelineBindPoint::Graphics, attachments);
	renderpass->AddDependency(EuropaRenderPass::SubpassExternal, 0, EuropaPipelineStageColorAttachmentOutput, EuropaAccessNone, EuropaPipelineStageColorAttachmentOutput, EuropaAccessColorAttachmentWrite);
	renderpass->CreateRenderpass();

	EuropaGraphicsPipelineCreateInfo pipelineDesc{};

	EuropaShaderStageInfo stages[2] = {
		EuropaShaderStageInfo{ EuropaShaderStageFragment, shaderFragment, "main" },
		EuropaShaderStageInfo{ EuropaShaderStageVertex, shaderVertex, "main"}
	};

	pipelineDesc.shaderStageCount = 2;
	pipelineDesc.stages = stages;
	pipelineDesc.vertexInput.attributeBindingCount = 0;
	pipelineDesc.vertexInput.vertexBindingCount = 0;
	pipelineDesc.inputAssembly.topology = EuropaPrimitiveTopology::TriangleList;
	pipelineDesc.inputAssembly.primitiveRestartEnable = false;
	pipelineDesc.viewport.position = glm::vec2(0.0);
	pipelineDesc.viewport.size = swapChainCaps.surfaceCaps.currentExtent;
	pipelineDesc.viewport.minDepth = 0.0f;
	pipelineDesc.viewport.maxDepth = 1.0f;
	pipelineDesc.scissor.position = glm::vec2(0.0);
	pipelineDesc.scissor.size = swapChainCaps.surfaceCaps.currentExtent;
	pipelineDesc.rasterizer.depthClamp = false;
	pipelineDesc.rasterizer.counterClockwise = true;
	pipelineDesc.rasterizer.cullBackFace = false;
	pipelineDesc.rasterizer.cullFrontFace = false;
	pipelineDesc.rasterizer.rasterizerDiscard = false;
	pipelineDesc.layout = pipelineLayout;
	pipelineDesc.renderpass = renderpass;
	pipelineDesc.targetSubpass = 0;

	EuropaGraphicsPipeline* pipeline = selectedDevice->CreateGraphicsPipeline(pipelineDesc);

	std::vector<EuropaFramebuffer*> framebuffers;
	for (EuropaImageView* view : swapChainImageViews)
	{
		EuropaFramebufferCreateInfo desc;
		desc.attachments = { view };
		desc.width = swapChainCaps.surfaceCaps.currentExtent.x;
		desc.height = swapChainCaps.surfaceCaps.currentExtent.y;
		desc.layers = 1;
		desc.renderpass = renderpass;

		framebuffers.push_back(selectedDevice->CreateFramebuffer(desc));
	}

	EuropaCommandPool* cmdpool = selectedDevice->CreateCommandPool(requiredQueues[0]);

	std::vector<EuropaCmdlist*> cmdlists = cmdpool->AllocateCommandBuffers(0, uint32(swapChainImages.size()));

	uint32 i = 0;
	for (EuropaCmdlist* cmdlist : cmdlists)
	{
		cmdlist->Begin();
		cmdlist->BeginRenderpass(renderpass, framebuffers[i], glm::ivec2(0), glm::uvec2(swapChainCaps.surfaceCaps.currentExtent), 1, glm::vec4(0.0, 0.0, 0.0, 1.0));
		cmdlist->BindPipeline(pipeline);
		cmdlist->DrawInstanced(3, 1, 0, 0);
		cmdlist->EndRenderpass();
		cmdlist->End();
		i++;
	}

	const int MAX_FRAMES_IN_FLIGHT = 3;

	std::vector<EuropaSemaphore*> imageAvailableSemaphore;
	std::vector<EuropaSemaphore*> renderFinishedSemaphore;
	std::vector<EuropaFence*> inFlightFences;
	std::vector<EuropaFence*> imagesInFlight;
	imagesInFlight.resize(swapChainImages.size(), nullptr);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		imageAvailableSemaphore.push_back(selectedDevice->CreateSema());
		renderFinishedSemaphore.push_back(selectedDevice->CreateSema());
		inFlightFences.push_back(selectedDevice->CreateFence(true));
	}

	size_t currentFrame = 0;
	size_t frames = 0;
	bool running = true;

	// Rendering thread
	std::thread rendering([&]()
	{
		while (running)
		{
			selectedDevice->WaitForFences(1, &inFlightFences[currentFrame]);

			uint32 imageIndex = swapChain->AcquireNextImage(imageAvailableSemaphore[currentFrame]);

			if (imagesInFlight[imageIndex] != nullptr)  selectedDevice->WaitForFences(1, &imagesInFlight[imageIndex]);
			imagesInFlight[imageIndex] = inFlightFences[currentFrame];

			selectedDevice->ResetFences(1, &inFlightFences[currentFrame]);

			EuropaPipelineStage waitStage = EuropaPipelineStageColorAttachmentOutput;
			cmdQueue->Submit(1, &imageAvailableSemaphore[currentFrame], &waitStage, 1, &cmdlists[imageIndex], 1, &renderFinishedSemaphore[currentFrame], inFlightFences[currentFrame]);
			cmdQueue->Present(1, &renderFinishedSemaphore[currentFrame], 1, &swapChain, imageIndex);

			currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		}
	});

	// Window thread (main thread)
	s.Run([]() {});

	running = false;
	rendering.join();

	selectedDevice->WaitIdle();

	for (auto f : inFlightFences) GanymedeDelete(f);
	for (auto s : imageAvailableSemaphore) GanymedeDelete(s);
	for (auto s : renderFinishedSemaphore) GanymedeDelete(s);
	GanymedeDelete(cmdpool);
	for (auto fb : framebuffers) GanymedeDelete(fb);
	GanymedeDelete(pipeline);
	GanymedeDelete(renderpass);
	GanymedeDelete(pipelineLayout);
	GanymedeDelete(shaderFragment);
	GanymedeDelete(shaderVertex);
	GanymedeDelete(swapChain);
	GanymedeDelete(surface);
	GanymedeDelete(cmdQueue);

	return 0;
}