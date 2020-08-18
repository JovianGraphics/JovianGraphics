#pragma once

#include "Europa.h"

#ifdef EUROPA_VULKAN

#include <vulkan/vulkan.h>

VkFormat EuropaImageFormat2VkFormat(EuropaImageFormat f);
EuropaImageFormat VkFormat2EuropaImageFormat(VkFormat f);

inline VkColorSpaceKHR EuropaColorSpace2VkColorSpaceKHR(EuropaColorSpace c) { return VkColorSpaceKHR(c); }
inline EuropaColorSpace VkColorSpaceKHR2EuropaColorSpace(VkColorSpaceKHR c) { return EuropaColorSpace(c); }

inline VkPresentModeKHR EuropaPresentMode2VkPresentModeKHR(EuropaPresentMode m) { return VkPresentModeKHR(m); }
inline EuropaPresentMode VkPresentModeKHR2EuropaPresentMode(VkPresentModeKHR m) { return EuropaPresentMode(m); }

class EuropaSurfaceVk : public EuropaSurface
{
public:
	~EuropaSurfaceVk() {};

	VkSurfaceKHR m_surface;
};

class EuropaDeviceVk : public EuropaDevice
{
public:
	VkPhysicalDevice m_phyDevice = nullptr;
	VkDevice m_device = nullptr;

	EuropaDeviceType GetType();
	std::string GetName();
	std::vector<EuropaQueueFamilyProperties> GetQueueFamilies(EuropaSurface* surface);
	void CreateLogicalDevice(uint32 queueFamilyCount, EuropaQueueFamilyProperties* queues, uint32* queueCount);
	EuropaQueue* GetQueue(EuropaQueueFamilyProperties& queue);
	EuropaSwapChainCapabilities getSwapChainCapabilities(EuropaSurface* surface);
	EuropaSwapChain* CreateSwapChain(EuropaSwapChainCreateInfo& args);
	std::vector<EuropaImage*> GetSwapChainImages(EuropaSwapChain* swapChain);
	EuropaImageView* CreateImageView(EuropaImageViewCreateInfo& args);
	EuropaFramebuffer* CreateFramebuffer(EuropaFramebufferCreateInfo& args);
	EuropaShaderModule* CreateShaderModule(const uint32* spvBinary, uint32 size);
	EuropaPipelineLayout* CreatePipelineLayout(EuropaPipelineLayoutInfo& args);
	EuropaRenderPass* CreateRenderPassBuilder();
	EuropaGraphicsPipeline* CreateGraphicsPipeline(EuropaGraphicsPipelineCreateInfo& args);
	EuropaCommandPool* CreateCommandPool(EuropaQueueFamilyProperties& queue);
	EuropaSemaphore* CreateSema();
	EuropaFence* CreateFence(bool createSignaled = false);
	void WaitIdle();
	void WaitForFences(uint32 numFences, EuropaFence** fences, bool waitAll = true, uint64 timeout = UINT64_MAX);
	void ResetFences(uint32 numFences, EuropaFence** fences);

	~EuropaDeviceVk();
};

class EuropaSwapChainVk : public EuropaSwapChain
{
public:
	EuropaDeviceVk* m_device;
	VkSwapchainKHR m_swapchain;

	uint32 AcquireNextImage(EuropaSemaphore* semaphore);

	~EuropaSwapChainVk();
};

class EuropaImageVk : public EuropaImage
{
public:
	VkImage m_image;
};

class EuropaImageViewVk : public EuropaImageView
{
public:
	EuropaDeviceVk* m_device;
	VkImageView m_view;

	~EuropaImageViewVk();
};

class EuropaFramebufferVk : public EuropaFramebuffer
{
public:
	EuropaDeviceVk* m_device;
	VkFramebuffer m_framebuffer;

	~EuropaFramebufferVk();
};

class EuropaShaderModuleVk : public EuropaShaderModule
{
public:
	EuropaDeviceVk* m_device;
	VkShaderModule m_shaderModule;

	~EuropaShaderModuleVk();
};

class EuropaQueueVk : public EuropaQueue
{
public:
	EuropaDeviceVk* m_device;
	VkQueue m_queue;

	void Submit(
		uint32 waitSemaphoreCount, EuropaSemaphore** waitSemaphores, EuropaPipelineStage* waitStages,
		uint32 cmdlistCount, EuropaCmdlist** cmdlists,
		uint32 signalSemaphoreCount, EuropaSemaphore** signalSemaphores, EuropaFence* fence = nullptr);
	void Present(uint32 waitSemaphoreCount, EuropaSemaphore** waitSemaphores, uint32 swapchainCount, EuropaSwapChain** swapchains, uint32 imageIndex);
	void WaitIdle();

	~EuropaQueueVk() {};
};

class EuropaCommandPoolVk : public EuropaCommandPool
{
public:
	EuropaDeviceVk* m_device;
	VkCommandPool m_pool;

	std::vector<EuropaCmdlist*> AllocateCommandBuffers(uint8 level, uint32 count);
	~EuropaCommandPoolVk();
};

class EuropaCmdlistVk : public EuropaCmdlist
{
public:
	EuropaDeviceVk* m_device;
	EuropaCommandPoolVk* m_pool;
	VkCommandBuffer m_cmdlist;

	void Begin();
	void End();
	void BeginRenderpass(EuropaRenderPass* renderpass, EuropaFramebuffer* framebuffer, glm::ivec2 offset, glm::uvec2 extent, uint32 clearValueCount, glm::vec4 clearColor);
	void EndRenderpass();
	void BindPipeline(EuropaGraphicsPipeline* pipeline);
	void DrawInstanced(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);

	~EuropaCmdlistVk() {};
};

class EuropaSemaphoreVk : public EuropaSemaphore
{
public:
	EuropaDeviceVk* m_device;
	VkSemaphore m_sema;

	virtual ~EuropaSemaphoreVk();
};

class EuropaFenceVk : public EuropaFence
{
public:
	EuropaDeviceVk* m_device;
	VkFence m_fence;

	virtual ~EuropaFenceVk();
};

class EuropaPipelineLayoutVk : public EuropaPipelineLayout
{
public:
	EuropaDeviceVk* m_device;
	VkPipelineLayout m_layout;

	~EuropaPipelineLayoutVk();
};

class EuropaRenderPassVk : public EuropaRenderPass
{
private:
	std::vector<VkAttachmentDescription> attachments;
	std::vector<VkAttachmentReference> attachmentReferences;
	std::vector<VkSubpassDescription> subpasses;
	std::vector<VkSubpassDependency> dependencies;

public:
	EuropaDeviceVk* m_device;
	VkRenderPass m_renderPass;
	
	uint32 AddAttachment(EuropaAttachmentInfo& attachment);
	uint32 AddSubpass(EuropaPipelineBindPoint bindPoint, std::vector<EuropaAttachmentReference>& attachments);
	void AddDependency(uint32 srcPass, uint32 dstPass, EuropaPipelineStage srcStage, EuropaAccess srcAccess, EuropaPipelineStage dstStage, EuropaAccess dstAccess);
	void CreateRenderpass();

	~EuropaRenderPassVk();
};

class EuropaGraphicsPipelineVk : public EuropaGraphicsPipeline
{
public:
	EuropaDeviceVk* m_device;
	VkPipeline m_pipeline;

	virtual ~EuropaGraphicsPipelineVk();
};

class EuropaVk : public Europa
{
private:
	VkDebugUtilsMessengerEXT debugMessenger;

	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void SetupDebugMessenger();

public:
	VkInstance m_instance;

	EuropaVk();
	~EuropaVk();

	std::vector<EuropaDevice*> GetDevices();
	EuropaSurface* CreateSurface(IoSurface* ioSurface);
};

#endif