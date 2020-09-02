#include "Io/Source/Io.h"
#include "Io/Source/IoEntryFunc.h"
#include "Europa/Source/Europa.h"
#include "Europa/Source/EuropaVk.h"
#include "Europa/Source/EuropaUtils.h"
#include "Ganymede/Source/Ganymede.h"

#include "triangle.frag.h"
#include "triangle.vert.h"

#include <thread>
#include <chrono>

struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;
};

int AppMain(IoSurface& s)
{
	// Create Europa Instance
	Europa& europa = EuropaVk();

	// Enumerate and select a Device
	std::vector<EuropaDevice::Ref> devices = europa.GetDevices();
	for (EuropaDevice::Ref d : devices)
	{
		GanymedePrint d->GetName(), ":", EuropaDeviceTypeToString(d->GetType());
	}

	EuropaDevice::Ref selectedDevice = devices[0];

	// Create Surface and its respective Queues
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

	// Create Swapchain
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
	swapChainCreateInfo.presentMode = EuropaPresentMode::Mailbox;
	swapChainCreateInfo.surface = surface;
	swapChainCreateInfo.surfaceTransform = swapChainCaps.surfaceCaps.currentTransform;

	EuropaSwapChain* swapChain = selectedDevice->CreateSwapChain(swapChainCreateInfo);

	// Create swap chain Images & Image Views
	std::vector<EuropaImage::Ref> swapChainImages = selectedDevice->GetSwapChainImages(swapChain);
	std::vector<EuropaImageView::Ref> swapChainImageViews;

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

	// Create Renderpass
	EuropaRenderPass* renderpass = selectedDevice->CreateRenderPassBuilder();
	uint32 presentTarget = renderpass->AddAttachment(EuropaAttachmentInfo{
		EuropaImageFormat::BGRA8sRGB,
		EuropaAttachmentLoadOp::Clear,
		EuropaAttachmentStoreOp::Store,
		EuropaAttachmentLoadOp::DontCare,
		EuropaAttachmentStoreOp::DontCare,
		EuropaImageLayout::Undefined,
		EuropaImageLayout::Present
	});
	std::vector<EuropaAttachmentReference> attachments = { { presentTarget, EuropaImageLayout::ColorAttachment } };
	uint32 forwardPass = renderpass->AddSubpass(EuropaPipelineBindPoint::Graphics, attachments);
	renderpass->AddDependency(EuropaRenderPass::SubpassExternal, forwardPass, EuropaPipelineStageColorAttachmentOutput, EuropaAccessNone, EuropaPipelineStageColorAttachmentOutput, EuropaAccessColorAttachmentWrite);
	renderpass->CreateRenderpass();

	// Create Framebuffers
	std::vector<EuropaFramebuffer*> framebuffers;
	for (EuropaImageView::Ref view : swapChainImageViews)
	{
		EuropaFramebufferCreateInfo desc;
		desc.attachments = { view };
		desc.width = swapChainCaps.surfaceCaps.currentExtent.x;
		desc.height = swapChainCaps.surfaceCaps.currentExtent.y;
		desc.layers = 1;
		desc.renderpass = renderpass;

		framebuffers.push_back(selectedDevice->CreateFramebuffer(desc));
	}

	// Create Pipeline
	EuropaVertexInputBindingInfo binding;
	binding.binding = 0;
	binding.stride = sizeof(Vertex);
	binding.perInstance = false;

	EuropaVertexAttributeBindingInfo attributes[2];
	attributes[0].binding = 0;
	attributes[0].location = 0;
	attributes[0].offset = offsetof(Vertex, Vertex::pos);
	attributes[0].format = EuropaImageFormat::RG32F;

	attributes[1].binding = 0;
	attributes[1].location = 1;
	attributes[1].offset = offsetof(Vertex, Vertex::color);
	attributes[1].format = EuropaImageFormat::RGB32F;

	EuropaShaderModule* shaderFragment = selectedDevice->CreateShaderModule(shader_spv_triangle_frag, sizeof(shader_spv_triangle_frag));
	EuropaShaderModule* shaderVertex = selectedDevice->CreateShaderModule(shader_spv_triangle_vert, sizeof(shader_spv_triangle_vert));

	EuropaDescriptorSetLayout* descLayout = selectedDevice->CreateDescriptorSetLayout();
	descLayout->UniformBuffer(0, 1, EuropaShaderStageAllGraphics);
	descLayout->Build();

	EuropaPipelineLayout* pipelineLayout = selectedDevice->CreatePipelineLayout(EuropaPipelineLayoutInfo{ 1, 0, &descLayout });

	EuropaGraphicsPipelineCreateInfo pipelineDesc{};

	EuropaShaderStageInfo stages[2] = {
		EuropaShaderStageInfo{ EuropaShaderStageFragment, shaderFragment, "main" },
		EuropaShaderStageInfo{ EuropaShaderStageVertex, shaderVertex, "main"}
	};

	pipelineDesc.shaderStageCount = 2;
	pipelineDesc.stages = stages;
	pipelineDesc.vertexInput.vertexBindingCount = 1;
	pipelineDesc.vertexInput.vertexBindings = &binding;
	pipelineDesc.vertexInput.attributeBindingCount = 2;
	pipelineDesc.vertexInput.attributeBindings = attributes;
	pipelineDesc.viewport.position = glm::vec2(0.0);
	pipelineDesc.viewport.size = swapChainCaps.surfaceCaps.currentExtent;
	pipelineDesc.viewport.minDepth = 0.0f;
	pipelineDesc.viewport.maxDepth = 1.0f;
	pipelineDesc.scissor.position = glm::vec2(0.0);
	pipelineDesc.scissor.size = swapChainCaps.surfaceCaps.currentExtent;
	pipelineDesc.layout = pipelineLayout;
	pipelineDesc.renderpass = renderpass;
	pipelineDesc.targetSubpass = forwardPass;

	EuropaGraphicsPipeline* pipeline = selectedDevice->CreateGraphicsPipeline(pipelineDesc);

	// Create Command Pools and Command Lists
	EuropaCommandPool* cmdpool = selectedDevice->CreateCommandPool(cmdQueue);

	std::vector<EuropaCmdlist*> cmdlists = cmdpool->AllocateCommandBuffers(0, uint32(swapChainImages.size()));

	// Create Utils
	EuropaCmdlist* copyCmdlist = cmdpool->AllocateCommandBuffers(0, 1)[0];
	EuropaTransferUtil* transferUtil = new EuropaTransferUtil(selectedDevice, cmdQueue, copyCmdlist, 64 << 20); // 64M

	// Geometry data
	const std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
		{{0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}},
		{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
	};

	const std::vector<uint16> indices = {
		0, 1, 2, 2, 3, 0
	};

	// Constants & Uniform buffers & Descriptor Pools / Sets
	struct ShaderConstants {
		glm::vec4 color;
	};

	EuropaDescriptorPoolSizes descPoolSizes;
	descPoolSizes.Uniform = uint32(swapChainImages.size());

	EuropaDescriptorPool* pool = selectedDevice->CreateDescriptorPool(descPoolSizes, uint32(swapChainImages.size()));

	std::vector<EuropaDescriptorSet*> descSets;

	for (uint32 i = 0; i < swapChainImages.size(); i++)
	{
		descSets.push_back(pool->AllocateDescriptorSet(descLayout));
	}

	uint32 constantsSize = alignUp(uint32(sizeof(ShaderConstants)), selectedDevice->GetMinUniformBufferOffsetAlignment());

	EuropaBufferInfo uniformBufferInfo;
	uniformBufferInfo.exclusive = true;
	uniformBufferInfo.size = uint32(swapChainImages.size() * constantsSize);
	uniformBufferInfo.usage = EuropaBufferUsage(EuropaBufferUsageUniform);
	uniformBufferInfo.memoryUsage = EuropaMemoryUsage::Cpu2Gpu;
	EuropaBuffer* uniformBuffer = selectedDevice->CreateBuffer(uniformBufferInfo);

	{
		uint8* mappedBuffer = uniformBuffer->Map<uint8>();
		for (uint32 i = 0; i < swapChainImages.size(); i++)
		{
			reinterpret_cast<ShaderConstants*>(mappedBuffer + i * constantsSize)->color = glm::vec4(1.0, 1.0, 1.0, 1.0);
		}
		uniformBuffer->Unmap();
	}

	for (uint32 i = 0; i < swapChainImages.size(); i++)
	{
		descSets[i]->SetUniformBuffer(uniformBuffer, constantsSize * i, sizeof(ShaderConstants), 0, 0);
	}

	// Create & Upload geometry buffers
	EuropaBufferInfo vertexBufferInfo;
	vertexBufferInfo.exclusive = true;
	vertexBufferInfo.size = uint32(vertices.size() * sizeof(Vertex));
	vertexBufferInfo.usage = EuropaBufferUsage(EuropaBufferUsageVertex | EuropaBufferUsageTransferDst);
	vertexBufferInfo.memoryUsage = EuropaMemoryUsage::GpuOnly;
	EuropaBuffer* vertexBuffer = selectedDevice->CreateBuffer(vertexBufferInfo);
	
	transferUtil->UploadToBufferEx(vertexBuffer, vertices.data(), uint32(vertices.size()));

	EuropaBufferInfo indexBufferInfo;
	indexBufferInfo.exclusive = true;
	indexBufferInfo.size = uint32(indices.size() * sizeof(uint32));
	indexBufferInfo.usage = EuropaBufferUsage(EuropaBufferUsageIndex | EuropaBufferUsageTransferDst);
	indexBufferInfo.memoryUsage = EuropaMemoryUsage::GpuOnly;
	EuropaBuffer* indexBuffer = selectedDevice->CreateBuffer(indexBufferInfo);

	transferUtil->UploadToBufferEx(indexBuffer, indices.data(), uint32(indices.size()));

	// Record Command Lists
	uint32 i = 0;
	for (EuropaCmdlist* cmdlist : cmdlists)
	{
		EuropaClearValue clearValue;
		clearValue.color = glm::vec4(0.0, 0.0, 0.0, 1.0);

		cmdlist->Begin();
		cmdlist->BeginRenderpass(renderpass, framebuffers[i], glm::ivec2(0), glm::uvec2(swapChainCaps.surfaceCaps.currentExtent), 1, &clearValue);
		cmdlist->BindPipeline(pipeline);
		cmdlist->BindVertexBuffer(vertexBuffer, 0, 0);
		cmdlist->BindIndexBuffer(indexBuffer, 0, EuropaImageFormat::R16UI);
		cmdlist->BindDescriptorSets(EuropaPipelineBindPoint::Graphics, pipelineLayout, descSets[i]);
		cmdlist->DrawIndexed(6, 1, 0, 0, 0);
		cmdlist->EndRenderpass();
		cmdlist->End();
		i++;
	}

	// Create Synchronization primitives for Present
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
		auto startTime = std::chrono::steady_clock::now();
		while (running)
		{
			selectedDevice->WaitForFences(1, &inFlightFences[currentFrame]);

			uint32 imageIndex = swapChain->AcquireNextImage(imageAvailableSemaphore[currentFrame]);

			if (imagesInFlight[imageIndex] != nullptr)  selectedDevice->WaitForFences(1, &imagesInFlight[imageIndex]);
			imagesInFlight[imageIndex] = inFlightFences[currentFrame];

			selectedDevice->ResetFences(1, &inFlightFences[currentFrame]);

			auto currentTime = std::chrono::steady_clock::now();
			float runTime = std::chrono::duration<float>(currentTime - startTime).count();

			ShaderConstants* constants = reinterpret_cast<ShaderConstants*>(uniformBuffer->Map<uint8>() + currentFrame * constantsSize);
			constants->color = glm::vec4(sin(runTime) * 0.5 + 0.5, cos(runTime) * 0.5 + 0.5, cos(runTime * 2.0) * 0.5 + 0.5, 1.0);
			uniformBuffer->Unmap();

			EuropaPipelineStage waitStage = EuropaPipelineStageColorAttachmentOutput;
			cmdQueue->Submit(1, &imageAvailableSemaphore[currentFrame], &waitStage, 1, &cmdlists[imageIndex], 1, &renderFinishedSemaphore[currentFrame], inFlightFences[currentFrame]);
			cmdQueue->Present(1, &renderFinishedSemaphore[currentFrame], 1, &swapChain, imageIndex);

			currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		}
	});

	// Window thread (main thread)
	s.Run([]() {});

	// Clean-up
	running = false;
	rendering.join();

	selectedDevice->WaitIdle();

	GanymedeDelete(transferUtil);
	GanymedeDelete(vertexBuffer);
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