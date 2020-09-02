#pragma once

#include "Europa.h"

#ifdef EUROPA_VULKAN

#include <vulkan/vulkan.h>
#include "External/VulkanMemoryAllocator/src/vk_mem_alloc.h"

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

class EuropaDeviceVk : public EuropaDevice, public std::enable_shared_from_this<EuropaDeviceVk>
{
public:
	typedef std::shared_ptr<EuropaDeviceVk> Ref;

	VkInstance& m_instance;
	VmaAllocator m_allocator;
	VkPhysicalDeviceProperties m_properties;

	VkPhysicalDevice m_phyDevice = nullptr;
	VkDevice m_device = nullptr;

	EuropaDeviceType GetType();
	std::string GetName();
	std::vector<EuropaQueueFamilyProperties> GetQueueFamilies(EuropaSurface* surface);
	void CreateLogicalDevice(uint32 queueFamilyCount, EuropaQueueFamilyProperties* queues, uint32* queueCount);
	EuropaQueue* GetQueue(EuropaQueueFamilyProperties& queue);
	EuropaSwapChainCapabilities getSwapChainCapabilities(EuropaSurface* surface);
	EuropaSwapChain* CreateSwapChain(EuropaSwapChainCreateInfo& args);
	std::vector<EuropaImage::Ref> GetSwapChainImages(EuropaSwapChain* swapChain);
	EuropaImage::Ref CreateImage(EuropaImageInfo& args);
	EuropaImageView::Ref CreateImageView(EuropaImageViewCreateInfo& args);
	EuropaFramebuffer* CreateFramebuffer(EuropaFramebufferCreateInfo& args);
	EuropaShaderModule* CreateShaderModule(const uint32* spvBinary, uint32 size);
	EuropaDescriptorSetLayout* CreateDescriptorSetLayout();
	EuropaDescriptorPool* CreateDescriptorPool(EuropaDescriptorPoolSizes& sizes, uint32 maxSets);
	EuropaPipelineLayout* CreatePipelineLayout(EuropaPipelineLayoutInfo& args);
	EuropaRenderPass* CreateRenderPassBuilder();
	EuropaGraphicsPipeline* CreateGraphicsPipeline(EuropaGraphicsPipelineCreateInfo& args);
	EuropaCommandPool* CreateCommandPool(EuropaQueue* queue);
	EuropaSemaphore* CreateSema();
	EuropaFence* CreateFence(bool createSignaled = false);
	void WaitIdle();
	void WaitForFences(uint32 numFences, EuropaFence** fences, bool waitAll = true, uint64 timeout = UINT64_MAX);
	void ResetFences(uint32 numFences, EuropaFence** fences);
	EuropaBuffer* CreateBuffer(EuropaBufferInfo& args);

	uint32 GetMinUniformBufferOffsetAlignment();

	EuropaDeviceVk(VkInstance& instance);
	~EuropaDeviceVk();
};

class EuropaSwapChainVk : public EuropaSwapChain
{
public:
	EuropaDeviceVk::Ref m_device;
	VkSwapchainKHR m_swapchain;

	int32 AcquireNextImage(EuropaSemaphore* semaphore);

	~EuropaSwapChainVk();
};

class EuropaBufferVk : public EuropaBuffer
{
public:
	EuropaDeviceVk::Ref m_device;
	EuropaBufferInfo m_info;
	VkBuffer m_buffer;
	VmaAllocation m_allocation;
	
	void* MapT();
	void Unmap();
	EuropaBufferInfo GetInfo();

	~EuropaBufferVk();
};

class EuropaImageVk : public EuropaImage, public std::enable_shared_from_this<EuropaImageVk>
{
public:
	typedef std::shared_ptr<EuropaImageVk> Ref;

	EuropaDeviceVk::Ref m_device;
	VkImage m_image;
	VmaAllocation m_alloc;

	bool external = true;

	EuropaImageVk() : std::enable_shared_from_this<EuropaImageVk>() {};
	~EuropaImageVk();
};

class EuropaImageViewVk : public EuropaImageView, public std::enable_shared_from_this<EuropaImageViewVk>
{
public:
	typedef std::shared_ptr<EuropaImageViewVk> Ref;

	EuropaDeviceVk::Ref m_device;
	VkImageView m_view;

	EuropaImageViewVk() : std::enable_shared_from_this<EuropaImageViewVk>() {};
	~EuropaImageViewVk();
};

class EuropaFramebufferVk : public EuropaFramebuffer
{
public:
	EuropaDeviceVk::Ref m_device;
	VkFramebuffer m_framebuffer;

	~EuropaFramebufferVk();
};

class EuropaShaderModuleVk : public EuropaShaderModule
{
public:
	EuropaDeviceVk::Ref m_device;
	VkShaderModule m_shaderModule;

	~EuropaShaderModuleVk();
};

class EuropaQueueVk : public EuropaQueue
{
public:
	EuropaDeviceVk::Ref m_device;
	VkQueue m_queue;

	void Submit(
		uint32 waitSemaphoreCount, EuropaSemaphore** waitSemaphores, EuropaPipelineStage* waitStages,
		uint32 cmdlistCount, EuropaCmdlist** cmdlists,
		uint32 signalSemaphoreCount, EuropaSemaphore** signalSemaphores, EuropaFence* fence = nullptr);
	void Submit(EuropaCmdlist* cmdlist);
	void Present(uint32 waitSemaphoreCount, EuropaSemaphore** waitSemaphores, uint32 swapchainCount, EuropaSwapChain** swapchains, uint32 imageIndex);
	void WaitIdle();

	~EuropaQueueVk() {};
};

class EuropaDescriptorSetVk : public EuropaDescriptorSet
{
public:
	EuropaDeviceVk::Ref m_device;
	VkDescriptorSet m_set;

	void SetUniformBuffer(EuropaBuffer* buffer, uint32 offset, uint32 size, uint32 binding, uint32 arrayElement);

	~EuropaDescriptorSetVk() {};
};

class EuropaDescriptorPoolVk : public EuropaDescriptorPool
{
public:
	EuropaDeviceVk::Ref m_device;
	VkDescriptorPool m_pool;

	EuropaDescriptorSet* AllocateDescriptorSet(EuropaDescriptorSetLayout* layout);

	~EuropaDescriptorPoolVk();
};

class EuropaCommandPoolVk : public EuropaCommandPool
{
public:
	EuropaDeviceVk::Ref m_device;
	VkCommandPool m_pool;

	std::vector<EuropaCmdlist*> AllocateCommandBuffers(uint8 level, uint32 count);
	~EuropaCommandPoolVk();
};

class EuropaCmdlistVk : public EuropaCmdlist
{
public:
	EuropaDeviceVk::Ref m_device;
	EuropaCommandPoolVk* m_pool;
	VkCommandBuffer m_cmdlist;

	void Begin();
	void End();
	void BeginRenderpass(EuropaRenderPass* renderpass, EuropaFramebuffer* framebuffer, glm::ivec2 offset, glm::uvec2 extent, uint32 clearValueCount, EuropaClearValue* clearColor);
	void EndRenderpass();
	void BindPipeline(EuropaGraphicsPipeline* pipeline);
	void DrawInstanced(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);
	void DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 firstVertex, uint32 firstInstance);
	void BindVertexBuffer(EuropaBuffer* buffer, uint32 offset, uint32 binding);
	void BindIndexBuffer(EuropaBuffer* buffer, uint32 offset, EuropaImageFormat indexFormat);
	void CopyBuffer(EuropaBuffer* dst, EuropaBuffer* src, uint32 size, uint32 srcOffset = 0, uint32 dstOffset = 0);
	void BindDescriptorSets(EuropaPipelineBindPoint bindPoint, EuropaPipelineLayout* layout, EuropaDescriptorSet* descSet, uint32 set = 0);

	~EuropaCmdlistVk() {};
};

class EuropaSemaphoreVk : public EuropaSemaphore
{
public:
	EuropaDeviceVk::Ref m_device;
	VkSemaphore m_sema;

	virtual ~EuropaSemaphoreVk();
};

class EuropaFenceVk : public EuropaFence
{
public:
	EuropaDeviceVk::Ref m_device;
	VkFence m_fence;

	virtual ~EuropaFenceVk();
};

class EuropaDescriptorSetLayoutVk : public EuropaDescriptorSetLayout
{
public:
	EuropaDeviceVk::Ref m_device;
	
	std::vector<VkDescriptorSetLayoutBinding> m_bindings;
	VkDescriptorSetLayout m_setLayout;

	void Build();
	void Clear();
	void UniformBuffer(uint32 binding, uint32 count, EuropaShaderStage stage);

	~EuropaDescriptorSetLayoutVk();
};

class EuropaPipelineLayoutVk : public EuropaPipelineLayout
{
public:
	EuropaDeviceVk::Ref m_device;
	VkPipelineLayout m_layout;

	~EuropaPipelineLayoutVk();
};

class EuropaRenderPassVk : public EuropaRenderPass
{
private:
	std::vector<VkAttachmentDescription> attachments;
	std::vector<VkAttachmentReference> attachmentReferences;
	std::vector<VkAttachmentReference> depthAttachmentReferences;
	std::vector<VkSubpassDescription> subpasses;
	std::vector<VkSubpassDependency> dependencies;

public:
	EuropaDeviceVk::Ref m_device;
	VkRenderPass m_renderPass;
	
	uint32 AddAttachment(EuropaAttachmentInfo& attachment);
	uint32 AddSubpass(EuropaPipelineBindPoint bindPoint, std::vector<EuropaAttachmentReference>& attachments, EuropaAttachmentReference* depthAttachment = nullptr);
	void AddDependency(uint32 srcPass, uint32 dstPass, EuropaPipelineStage srcStage, EuropaAccess srcAccess, EuropaPipelineStage dstStage, EuropaAccess dstAccess);
	void CreateRenderpass();

	~EuropaRenderPassVk();
};

class EuropaGraphicsPipelineVk : public EuropaGraphicsPipeline
{
public:
	EuropaDeviceVk::Ref m_device;
	VkPipeline m_pipeline;

	virtual ~EuropaGraphicsPipelineVk();
};

class EuropaVk : public Europa, public std::enable_shared_from_this<EuropaVk>
{
private:
	VkDebugUtilsMessengerEXT debugMessenger;

	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void SetupDebugMessenger();

public:
	typedef std::shared_ptr<Europa> Ref;

	VkInstance m_instance;

	EuropaVk();
	~EuropaVk();

	std::vector<EuropaDevice::Ref> GetDevices();
	EuropaSurface* CreateSurface(IoSurface* ioSurface);
};

#endif