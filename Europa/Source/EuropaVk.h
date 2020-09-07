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

class EuropaSurfaceVk : public EuropaSurface, public SHARE(EuropaSurfaceVk)
{
public:
	DECL_REF(EuropaSurfaceVk)

	VkSurfaceKHR m_surface;

	EuropaSurfaceVk() : SHARE(EuropaSurfaceVk)() {}
	~EuropaSurfaceVk() {};
};

class EuropaDeviceVk : public EuropaDevice, public SHARE(EuropaDeviceVk)
{
public:
	DECL_REF(EuropaDeviceVk)

	VkInstance& m_instance;
	VmaAllocator m_allocator;
	VkPhysicalDeviceProperties m_properties;

	VkPhysicalDevice m_phyDevice = nullptr;
	VkDevice m_device = nullptr;

	EuropaDeviceType GetType();
	std::string GetName();
	std::vector<EuropaQueueFamilyProperties> GetQueueFamilies(EuropaSurface::Ref surface);
	void CreateLogicalDevice(uint32 queueFamilyCount, EuropaQueueFamilyProperties* queues, uint32* queueCount);
	EuropaQueue::Ref GetQueue(EuropaQueueFamilyProperties& queue);
	EuropaSwapChainCapabilities getSwapChainCapabilities(EuropaSurface::Ref surface);
	EuropaSwapChain::Ref CreateSwapChain(EuropaSwapChainCreateInfo& args);
	std::vector<EuropaImage::Ref> GetSwapChainImages(EuropaSwapChain::Ref swapChain);
	EuropaImage::Ref CreateImage(EuropaImageInfo& args);
	EuropaImageView::Ref CreateImageView(EuropaImageViewCreateInfo& args);
	EuropaFramebuffer::Ref CreateFramebuffer(EuropaFramebufferCreateInfo& args);
	EuropaShaderModule::Ref CreateShaderModule(const uint32* spvBinary, uint32 size);
	EuropaDescriptorSetLayout::Ref CreateDescriptorSetLayout();
	EuropaDescriptorPool::Ref CreateDescriptorPool(EuropaDescriptorPoolSizes& sizes, uint32 maxSets);
	EuropaPipelineLayout::Ref CreatePipelineLayout(EuropaPipelineLayoutInfo& args);
	EuropaRenderPass::Ref CreateRenderPassBuilder();
	EuropaGraphicsPipeline::Ref CreateGraphicsPipeline(EuropaGraphicsPipelineCreateInfo& args);
	EuropaCommandPool::Ref CreateCommandPool(EuropaQueue::Ref queue);
	EuropaSemaphore::Ref CreateSema();
	EuropaFence::Ref CreateFence(bool createSignaled = false);
	void WaitIdle();
	void WaitForFences(uint32 numFences, EuropaFence::Ref* fences, bool waitAll = true, uint64 timeout = UINT64_MAX);
	void ResetFences(uint32 numFences, EuropaFence::Ref* fences);
	EuropaBuffer::Ref CreateBuffer(EuropaBufferInfo& args);
	EuropaBufferView::Ref CreateBufferView(EuropaBuffer::Ref buffer, uint32 size, uint32 offset = 0, EuropaImageFormat format = EuropaImageFormat::Undefined);

	uint32 GetMinUniformBufferOffsetAlignment();

	EuropaDeviceVk(VkInstance& instance);
	~EuropaDeviceVk();
};

class EuropaSwapChainVk : public EuropaSwapChain, public SHARE(EuropaSwapChainVk)
{
public:
	DECL_REF(EuropaSwapChainVk)

	EuropaDeviceVk::Ref m_device;
	VkSwapchainKHR m_swapchain;

	int32 AcquireNextImage(EuropaSemaphore::Ref semaphore);

	EuropaSwapChainVk() : SHARE(EuropaSwapChainVk)() {}
	~EuropaSwapChainVk();
};

class EuropaBufferVk : public EuropaBuffer, public SHARE(EuropaBufferVk)
{
public:
	DECL_REF(EuropaBufferVk)

	EuropaDeviceVk::Ref m_device;
	EuropaBufferInfo m_info;
	VkBuffer m_buffer;
	VmaAllocation m_allocation;
	
	void* MapT();
	void Unmap();
	EuropaBufferInfo GetInfo();

	EuropaBufferVk() : SHARE(EuropaBufferVk)() {}
	~EuropaBufferVk();
};

class EuropaBufferViewVk : public EuropaBufferView, public SHARE(EuropaBufferViewVk)
{
public:
	DECL_REF(EuropaBufferViewVk);

	EuropaDeviceVk::Ref m_device;
	EuropaBufferVk::Ref m_buffer;
	VkBufferView m_view;

	EuropaBufferViewVk() : SHARE(EuropaBufferViewVk)() {}
	~EuropaBufferViewVk();
};

class EuropaImageVk : public EuropaImage, public SHARE(EuropaImageVk)
{
public:
	DECL_REF(EuropaImageVk)

	EuropaDeviceVk::Ref m_device;
	VkImage m_image;
	VmaAllocation m_alloc;

	bool external = true;

	EuropaImageVk() : SHARE(EuropaImageVk)() {}
	~EuropaImageVk();
};

class EuropaImageViewVk : public EuropaImageView, public SHARE(EuropaImageViewVk)
{
public:
	DECL_REF(EuropaImageViewVk)

	EuropaDeviceVk::Ref m_device;
	VkImageView m_view;

	EuropaImageViewVk() : SHARE(EuropaImageViewVk)() {}
	~EuropaImageViewVk();
};

class EuropaFramebufferVk : public EuropaFramebuffer, public SHARE(EuropaFramebufferVk)
{
public:
	DECL_REF(EuropaFramebufferVk)

	EuropaDeviceVk::Ref m_device;
	VkFramebuffer m_framebuffer;

	EuropaFramebufferVk() : SHARE(EuropaFramebufferVk)() {}
	~EuropaFramebufferVk();
};

class EuropaShaderModuleVk : public EuropaShaderModule, public SHARE(EuropaShaderModuleVk)
{
public:
	DECL_REF(EuropaShaderModuleVk)

	EuropaDeviceVk::Ref m_device;
	VkShaderModule m_shaderModule;

	EuropaShaderModuleVk() : SHARE(EuropaShaderModuleVk)() {}
	~EuropaShaderModuleVk();
};

class EuropaQueueVk : public EuropaQueue, public SHARE(EuropaQueueVk)
{
public:
	DECL_REF(EuropaQueueVk)

	EuropaDeviceVk::Ref m_device;
	VkQueue m_queue;

	void Submit(
		uint32 waitSemaphoreCount, EuropaSemaphore::Ref* waitSemaphores, EuropaPipelineStage* waitStages,
		uint32 cmdlistCount, EuropaCmdlist::Ref* cmdlists,
		uint32 signalSemaphoreCount, EuropaSemaphore::Ref* signalSemaphores, EuropaFence::Ref fence = nullptr);
	void Submit(EuropaCmdlist::Ref cmdlist);
	void Present(uint32 waitSemaphoreCount, EuropaSemaphore::Ref* waitSemaphores, uint32 swapchainCount, EuropaSwapChain::Ref* swapchains, uint32 imageIndex);
	void WaitIdle();

	EuropaQueueVk() : SHARE(EuropaQueueVk)() {}
	~EuropaQueueVk() {};
};

class EuropaDescriptorSetVk : public EuropaDescriptorSet, public SHARE(EuropaDescriptorSetVk)
{
public:
	DECL_REF(EuropaDescriptorSetVk)

	EuropaDeviceVk::Ref m_device;
	EuropaDescriptorSetLayout::Ref m_layout;
	VkDescriptorSet m_set;

	void SetUniformBuffer(EuropaBuffer::Ref buffer, uint32 offset, uint32 size, uint32 binding, uint32 arrayElement);
	void SetUniformBufferDynamic(EuropaBuffer::Ref buffer, uint32 offset, uint32 size, uint32 binding, uint32 arrayElement);
	void SetBufferViewUniform(EuropaBufferView::Ref view, uint32 binding, uint32 arrayElement);
	void SetBufferViewStorage(EuropaBufferView::Ref view, uint32 binding, uint32 arrayElement);
	void SetImageViewStorage(EuropaImageView::Ref view, EuropaImageLayout layout, uint32 binding, uint32 arrayElement);
	void SetStorage(EuropaBuffer::Ref view, uint32 offset, uint32 size, uint32 binding, uint32 arrayElement);

	EuropaDescriptorSetVk() : SHARE(EuropaDescriptorSetVk)() {}
	~EuropaDescriptorSetVk() {};
};

class EuropaDescriptorPoolVk : public EuropaDescriptorPool, public SHARE(EuropaDescriptorPoolVk)
{
public:
	DECL_REF(EuropaDescriptorPoolVk)

	EuropaDeviceVk::Ref m_device;
	VkDescriptorPool m_pool;

	EuropaDescriptorSet::Ref AllocateDescriptorSet(EuropaDescriptorSetLayout::Ref layout);

	EuropaDescriptorPoolVk() : SHARE(EuropaDescriptorPoolVk)() {}
	~EuropaDescriptorPoolVk();
};

class EuropaCommandPoolVk : public EuropaCommandPool, public SHARE(EuropaCommandPoolVk)
{
public:
	DECL_REF(EuropaCommandPoolVk)

	EuropaDeviceVk::Ref m_device;
	VkCommandPool m_pool;

	std::vector<EuropaCmdlist::Ref> AllocateCommandBuffers(uint8 level, uint32 count);

	EuropaCommandPoolVk() : SHARE(EuropaCommandPoolVk)() {}
	~EuropaCommandPoolVk();
};

class EuropaCmdlistVk : public EuropaCmdlist, public SHARE(EuropaCmdlistVk)
{
public:
	DECL_REF(EuropaCmdlistVk)

	EuropaDeviceVk::Ref m_device;
	EuropaCommandPoolVk::Ref m_pool;
	VkCommandBuffer m_cmdlist;

	void Begin();
	void End();
	void BeginRenderpass(EuropaRenderPass::Ref renderpass, EuropaFramebuffer::Ref framebuffer, glm::ivec2 offset, glm::uvec2 extent, uint32 clearValueCount, EuropaClearValue* clearColor);
	void EndRenderpass();
	void BindPipeline(EuropaGraphicsPipeline::Ref pipeline);
	void DrawInstanced(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);
	void DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 firstVertex, uint32 firstInstance);
	void BindVertexBuffer(EuropaBuffer::Ref buffer, uint32 offset, uint32 binding);
	void BindIndexBuffer(EuropaBuffer::Ref buffer, uint32 offset, EuropaImageFormat indexFormat);
	void CopyBuffer(EuropaBuffer::Ref dst, EuropaBuffer::Ref src, uint32 size, uint32 srcOffset = 0, uint32 dstOffset = 0);
	void BindDescriptorSets(EuropaPipelineBindPoint bindPoint, EuropaPipelineLayout::Ref layout, EuropaDescriptorSet::Ref descSet, uint32 set = 0);
	void BindDescriptorSetsDynamicOffsets(EuropaPipelineBindPoint bindPoint, EuropaPipelineLayout::Ref layout, EuropaDescriptorSet::Ref descSet, uint32 set = 0, uint32 offset = 0);
	void Barrier(
		EuropaImage::Ref image,
		EuropaAccess beforeAccess, EuropaAccess afterAccess,
		EuropaImageLayout beforeLayout, EuropaImageLayout afterLayout,
		EuropaPipelineStage beforeStage = EuropaPipelineStageAllCommands, EuropaPipelineStage afterStage = EuropaPipelineStageAllCommands,
		EuropaQueue::Ref srcQueue = nullptr, EuropaQueue::Ref dstQueue = nullptr);
	void ClearImage(EuropaImage::Ref image, EuropaImageLayout layout, glm::vec4 color, uint32 baseMipLevel = 0, uint32 baseArrayLayer = 0, uint32 numMipLevls = 1, uint32 numArrayLayers = 1);

	EuropaCmdlistVk() : SHARE(EuropaCmdlistVk)() {}
	~EuropaCmdlistVk() {};
};

class EuropaSemaphoreVk : public EuropaSemaphore, public SHARE(EuropaSemaphoreVk)
{
public:
	DECL_REF(EuropaSemaphoreVk)

	EuropaDeviceVk::Ref m_device;
	VkSemaphore m_sema;

	EuropaSemaphoreVk() : SHARE(EuropaSemaphoreVk)() {}
	virtual ~EuropaSemaphoreVk();
};

class EuropaFenceVk : public EuropaFence, public SHARE(EuropaFenceVk)
{
public:
	DECL_REF(EuropaFenceVk)

	EuropaDeviceVk::Ref m_device;
	VkFence m_fence;

	EuropaFenceVk() : SHARE(EuropaFenceVk)() {}
	virtual ~EuropaFenceVk();
};

class EuropaDescriptorSetLayoutVk : public EuropaDescriptorSetLayout, public SHARE(EuropaDescriptorSetLayoutVk)
{
public:
	DECL_REF(EuropaDescriptorSetLayoutVk)

	EuropaDeviceVk::Ref m_device;
	
	std::vector<VkDescriptorSetLayoutBinding> m_bindings;
	VkDescriptorSetLayout m_setLayout;

	void Build();
	void Clear();
	void UniformBuffer(uint32 binding, uint32 count, EuropaShaderStage stage);
	void DynamicUniformBuffer(uint32 binding, uint32 count, EuropaShaderStage stage);
	void BufferViewUniform(uint32 binding, uint32 count, EuropaShaderStage stage);
	void BufferViewStorage(uint32 binding, uint32 count, EuropaShaderStage stage);
	void ImageViewStorage(uint32 binding, uint32 count, EuropaShaderStage stage);
	void Storage(uint32 binding, uint32 count, EuropaShaderStage stage);

	EuropaDescriptorSetLayoutVk() : SHARE(EuropaDescriptorSetLayoutVk)() {}
	~EuropaDescriptorSetLayoutVk();
};

class EuropaPipelineLayoutVk : public EuropaPipelineLayout, public SHARE(EuropaPipelineLayoutVk)
{
public:
	DECL_REF(EuropaPipelineLayoutVk)

	EuropaDeviceVk::Ref m_device;
	VkPipelineLayout m_layout;

	EuropaPipelineLayoutVk() : SHARE(EuropaPipelineLayoutVk)() {}
	~EuropaPipelineLayoutVk();
};

class EuropaRenderPassVk : public EuropaRenderPass, public SHARE(EuropaRenderPassVk)
{
private:
	std::vector<VkAttachmentDescription> attachments;
	std::vector<VkAttachmentReference> attachmentReferences;
	std::vector<VkAttachmentReference> depthAttachmentReferences;
	std::vector<VkSubpassDescription> subpasses;
	std::vector<VkSubpassDependency> dependencies;

public:
	DECL_REF(EuropaRenderPassVk)

	EuropaDeviceVk::Ref m_device;
	VkRenderPass m_renderPass;
	
	uint32 AddAttachment(EuropaAttachmentInfo& attachment);
	uint32 AddSubpass(EuropaPipelineBindPoint bindPoint, std::vector<EuropaAttachmentReference>& attachments, EuropaAttachmentReference* depthAttachment = nullptr);
	void AddDependency(uint32 srcPass, uint32 dstPass, EuropaPipelineStage srcStage, EuropaAccess srcAccess, EuropaPipelineStage dstStage, EuropaAccess dstAccess);
	void CreateRenderpass();

	EuropaRenderPassVk() : SHARE(EuropaRenderPassVk)() {}
	~EuropaRenderPassVk();
};

class EuropaGraphicsPipelineVk : public EuropaGraphicsPipeline, public SHARE(EuropaGraphicsPipelineVk)
{
public:
	DECL_REF(EuropaGraphicsPipelineVk)

	EuropaDeviceVk::Ref m_device;
	VkPipeline m_pipeline;

	EuropaGraphicsPipelineVk() : SHARE(EuropaGraphicsPipelineVk)() {}
	virtual ~EuropaGraphicsPipelineVk();
};

class EuropaVk : public Europa, public SHARE(EuropaVk)
{
private:
	VkDebugUtilsMessengerEXT debugMessenger;

	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void SetupDebugMessenger();

public:
	DECL_REF(EuropaVk)

	VkInstance m_instance;

	EuropaVk();
	~EuropaVk();

	std::vector<EuropaDevice::Ref> GetDevices();
	EuropaSurface::Ref CreateSurface(REF(IoSurface) ioSurface);
};

#endif