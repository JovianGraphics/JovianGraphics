#include "EuropaImGui.h"

#ifdef EUROPA_VULKAN
#include "EuropaVk.h"
#include <External/imgui/examples/imgui_impl_vulkan.h>
#endif

#ifdef IO_WIN32
#include <External/imgui/examples/imgui_impl_win32.h>
#endif

void EuropaImGui::Init()
{
}

void EuropaImGui::Render(uint32 frameId, EuropaCmdlist::Ref cmdlist)
{
	cmdlist->BeginRenderpass(m_renderPass, m_frameBuffers[frameId], glm::uvec2(0), m_viewportSize, 0, nullptr);
#ifdef EUROPA_VULKAN
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), std::static_pointer_cast<EuropaCmdlistVk>(cmdlist)->m_cmdlist);
#endif
	cmdlist->EndRenderpass();
}

void EuropaImGui::NewFrame()
{
#ifdef EUROPA_VULKAN
	ImGui_ImplVulkan_NewFrame();
#endif
#ifdef IO_WIN32
	ImGui_ImplWin32_NewFrame();
#endif
	ImGui::NewFrame();
}

void EuropaImGui::RebuildSwapChain(uint32 imageCount, glm::uvec2 imageSize, EuropaImageView::Ref* views, EuropaCmdlist::Ref cmdlist)
{
    m_frameBuffers.clear();

	m_viewportSize = imageSize;

	for (uint32 i = 0; i < imageCount; i++)
	{
		EuropaFramebufferCreateInfo desc;
		desc.attachments = { views[i] };
		desc.width = imageSize.x;
		desc.height = imageSize.y;
		desc.layers = 1;
		desc.renderpass = m_renderPass;

		EuropaFramebuffer::Ref framebuffer = m_device->CreateFramebuffer(desc);

		m_frameBuffers.push_back(framebuffer);
	}

#ifdef EUROPA_VULKAN
	ImGui_ImplVulkan_SetMinImageCount(imageCount);

	cmdlist->Begin();
	ImGui_ImplVulkan_CreateFontsTexture(std::static_pointer_cast<EuropaCmdlistVk>(cmdlist)->m_cmdlist);
	cmdlist->End();
	m_queue->Submit(cmdlist);
	m_queue->WaitIdle();
	ImGui_ImplVulkan_DestroyFontUploadObjects();
#endif
}

#ifdef IO_WIN32
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

EuropaImGui::EuropaImGui(Europa& europa, EuropaImageFormat imageFormat, EuropaDevice::Ref device, EuropaQueue::Ref queue, EuropaSurface::Ref surface, IoSurface::Ref _ioSurface)
	: m_device(device)
	, m_queue(queue)
{
#ifdef EUROPA_VULKAN
    EuropaVk& europaVk = (EuropaVk&)europa;
    EuropaDeviceVk::Ref deviceVk = std::static_pointer_cast<EuropaDeviceVk>(device);
    EuropaQueueVk::Ref queueVk = std::static_pointer_cast<EuropaQueueVk>(queue);
    EuropaSurfaceVk::Ref surfaceVk = std::static_pointer_cast<EuropaSurfaceVk>(surface);
#endif

	EuropaDescriptorPoolSizes poolSizes{};
	poolSizes.CombinedImageSampler = 8;
	poolSizes.InputAttachments = 8;
	poolSizes.SampledImage = 8;
	poolSizes.Sampler = 8;
	poolSizes.Storage = 8;
	poolSizes.StorageDynamic = 8;
	poolSizes.StorageImage = 8;
	poolSizes.StorageTexel = 8;
	poolSizes.Uniform = 8;
	poolSizes.UniformDynamic = 8;
	poolSizes.UniformTexel = 8;
	m_descPool = device->CreateDescriptorPool(poolSizes, 16);

	m_renderPass = device->CreateRenderPassBuilder();
	uint32 presentTarget = m_renderPass->AddAttachment(EuropaAttachmentInfo{
		imageFormat,
		EuropaAttachmentLoadOp::Load,
		EuropaAttachmentStoreOp::Store,
		EuropaAttachmentLoadOp::DontCare,
		EuropaAttachmentStoreOp::DontCare,
		EuropaImageLayout::ColorAttachment,
		EuropaImageLayout::Present
		});
	std::vector<EuropaAttachmentReference> attachments = { { presentTarget, EuropaImageLayout::ColorAttachment } };
	uint32 forwardPass = m_renderPass->AddSubpass(EuropaPipelineBindPoint::Graphics, attachments);
	m_renderPass->AddDependency(EuropaRenderPass::SubpassExternal, forwardPass, EuropaPipelineStageColorAttachmentOutput, EuropaAccessNone, EuropaPipelineStageColorAttachmentOutput, EuropaAccessColorAttachmentWrite);
	m_renderPass->CreateRenderpass();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	{
		ImPlotStyle& style = ImPlot::GetStyle();
		style.AntiAliasedLines = true;
		style.LineWeight = 2.0;
	}

	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameRounding = 4.0;
		style.FrameBorderSize = 2.0;
		style.WindowRounding = 4.0;
		style.WindowBorderSize = 1.0;
		style.ScrollbarSize = 16.0;
		
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02, 0.02, 0.02, 1.0));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.05, 0.05, 0.05, 1.0));
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.1, 0.1, 0.1, 1.0));
		ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.05, 0.05, 0.05, 1.0));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.01, 0.01, 0.01, 1.0));
		ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0);
		ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, ImVec4(0.05, 0.05, 0.05, 1.0));
		ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, 0);


		ImFontConfig config;
		config.OversampleH = 2;
		config.OversampleV = 1;
		config.GlyphExtraSpacing.x = 0.0f;
		io.Fonts->AddFontFromFileTTF("Assets/NotoSans-Regular.ttf", 16.0, &config);
	}

#ifdef EUROPA_VULKAN
	VkPipelineCache pipelineCache;
	{
		VkPipelineCacheCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		info.flags = 0;
		info.initialDataSize = 0;
		vkCreatePipelineCache(deviceVk->m_device, &info, nullptr, &pipelineCache);
	}

	{
		ImGui_ImplVulkan_InitInfo info{};
		info.Instance = europaVk.m_instance;
		info.PhysicalDevice = deviceVk->m_phyDevice;
		info.Device = deviceVk->m_device;
		info.QueueFamily = queueVk->m_property.queueIndex;
		info.Queue = queueVk->m_queue;
		info.PipelineCache = pipelineCache;
		info.DescriptorPool = std::static_pointer_cast<EuropaDescriptorPoolVk>(m_descPool)->m_pool;
		info.MinImageCount = 3;
		info.ImageCount = 3;
		info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		info.Allocator = nullptr;
		info.CheckVkResultFn = nullptr;

		ImGui_ImplVulkan_Init(&info, std::static_pointer_cast<EuropaRenderPassVk>(m_renderPass)->m_renderPass);
	}
#endif

#ifdef IO_WIN32
    IoSurfaceWin32::Ref ioSurface = std::static_pointer_cast<IoSurfaceWin32>(_ioSurface);
    ImGui_ImplWin32_Init(ioSurface->m_hwnd);

	ioSurface->m_externalCallback = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	};
	ioSurface->m_enableExternalCallback = true;
#endif
}

EuropaImGui::~EuropaImGui()
{
#ifdef EUROPA_VULKAN
	ImGui_ImplVulkan_Shutdown();
#endif
#ifdef IO_WIN32
    ImGui_ImplWin32_Shutdown();
#endif
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}
