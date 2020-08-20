#pragma once

#include "config.h"
#include "Ganymede/Source/Ganymede.h"

#include <vector>
#include <optional>

#include <glm/glm.hpp>

class IoSurface;

GANYMEDE_ENUM(EuropaDeviceType, 
	(Discrete)
	(Integrated)
	(Virtual)
	(CPU)
	(Other)
)

GANYMEDE_ENUM_FLAGS(EuropaQueueCapabilities,
	((Graphics, 0x00000001))
	((Compute, 0x00000002))
	((Transfer, 0x00000004))
	((SparseBinding, 0x00000008))
	((Protected, 0x00000010))
)

GANYMEDE_ENUM(EuropaImageFormat,
	(Undefined)

	(RG4Unorm)
	(RGBA4Unorm)

	(R5G6B5Unorm)
	(RGB5A1Unorm)
	
	(R8Unorm)
	(R8Snorm)
	(R8UI)
	(R8I)
	(R8sRGB)
	(RG8Unorm)
	(RG8Snorm)
	(RG8UI)
	(RG8I)
	(RG8sRGB)
	(RGB8Unorm)
	(RGB8Snorm)
	(RGB8UI)
	(RGB8I)
	(RGB8sRGB)
	(RGBA8Unorm)
	(RGBA8Snorm)
	(RGBA8UI)
	(RGBA8I)
	(RGBA8sRGB)

	(BGR8Unorm)
	(BGR8Snorm)
	(BGR8UI)
	(BGR8I)
	(BGR8sRGB)
	(BGRA8Unorm)
	(BGRA8Snorm)
	(BGRA8UI)
	(BGRA8I)
	(BGRA8sRGB)

	(R16Unorm)
	(R16Snorm)
	(R16UI)
	(R16I)
	(R16F)
	(RG16Unorm)
	(RG16Snorm)
	(RG16UI)
	(RG16I)
	(RG16F)
	(RGB16Unorm)
	(RGB16Snorm)
	(RGB16UI)
	(RGB16I)
	(RGB16F)
	(RGBA16Unorm)
	(RGBA16Snorm)
	(RGBA16UI)
	(RGBA16I)
	(RGBA16F)

	(R32UI)
	(R32I)
	(R32F)
	(RG32UI)
	(RG32I)
	(RG32F)
	(RGB32UI)
	(RGB32I)
	(RGB32F)
	(RGBA32UI)
	(RGBA32I)
	(RGBA32F)

	(B10G11R11UFloat)
	(E5B9G9R9UFloat)

	(BC1RGBUnorm)
	(BC1RGBsRGB)
	(BC1RGBAUnorm)
	(BC1RGBAsRGB)
	(BC2RGBAUnorm)
	(BC2RGBAsRGB)
	(BC3RGBAUnorm)
	(BC3RGBAsRGB)
	(BC4RGBAUnorm)
	(BC4RGBASnorm)
	(BC5RGBAUnorm)
	(BC5RGBASnorm)
)

GANYMEDE_ENUM_NUMBERED(EuropaColorSpace,
	((GammaSRGB, 0))
	((GammaP3, 1000104001))
	((LinearExtendedSRGB, 1000104002))
	((LinearDisplayP3, 1000104003))
	((GammaDCIP3, 1000104004))
	((LinearBT709, 1000104005))
	((GammaBT709, 1000104006))
	((LinearBT2020, 1000104007))
	((HDR10ST2084, 1000104008))
	((DolbyVision, 1000104009))
	((HDR10HLG, 1000104010))
	((LinearAdobeRGB, 1000104011))
	((GammaAdobeRGB, 1000104012))
	((PassThrough, 1000104013))
	((GammaExtendedSRGB, 1000104014))
	((DisplayNativeAMD, 1000213000))
)

GANYMEDE_ENUM_NUMBERED(EuropaPresentMode,
	((Immediate, 0))
	((Mailbox, 1))
	((FIFO, 2))
	((FIFORelaxed, 3))
)

GANYMEDE_ENUM_FLAGS(EuropaSurfaceTransform,
	((Identity, 0x00000001))
	((Rotate90, 0x00000002))
	((Rotate180, 0x00000004))
	((Rotate270, 0x00000008))
	((HorizontalMirror, 0x00000010))
	((HorizontalMirrorRotate90, 0x00000020))
	((HorizontalMirrorRotate180, 0x00000040))
	((HorizontalMirrorRotate270, 0x00000080))
	((Inherit, 0x00000100))
)

GANYMEDE_ENUM_NUMBERED(EuropaImageViewType,
	((View1D, 0))
	((View2D, 1))
	((View3D, 2))
	((ViewCube, 3))
	((View1DArray, 4))
	((View2DArray, 5))
	((ViewCubeArray, 6))
)

GANYMEDE_ENUM_FLAGS(EuropaShaderStage,
	((Vertex, 0x00000001))
	((TessControl, 0x00000002))
	((TessEval, 0x00000004))
	((Geometry, 0x00000008))
	((Fragment, 0x00000010))
	((Compute, 0x00000020))
	((AllGraphics, 0x0000001F))
	((All, 0x7FFFFFFF))
	((RayGen, 0x00000100))
	((AnyHit, 0x00000200))
	((ClosestHit, 0x00000400))
	((Miss, 0x00000800))
	((Intersection, 0x00001000))
	((Callable, 0x00002000))
	((Task, 0x00000040))
	((Mesh, 0x00000080))
)

GANYMEDE_ENUM_NUMBERED(EuropaPrimitiveTopology,
	((Points, 0))
	((LineList, 1))
	((LineStrip, 2))
	((TriangleList, 3))
	((TriangleStrip, 4))
	((TriangleFan, 5))
	((LineListWithAdjacency, 6))
	((LineStripWithAdjacency, 7))
	((TriangleListWithAdjacency, 8))
	((TriangleStripWithAdjacency, 9))
	((PatchList, 10))
)

GANYMEDE_ENUM_NUMBERED(EuropaAttachmentLoadOp,
	((Load, 0))
	((Clear, 1))
	((DontCare, 2))
)

GANYMEDE_ENUM_NUMBERED(EuropaAttachmentStoreOp,
	((Store, 0))
	((DontCare, 1))
)

GANYMEDE_ENUM_NUMBERED(EuropaImageLayout,
	((Undefined, 0))
	((General, 1))
	((ColorAttachment, 2))
	((DepthStencilAttachment, 3))
	((DepthStencilReadOnly, 4))
	((ShaderReadOnly, 5))
	((TransferSrc, 6))
	((TransferDst, 7))
	((Preinitialized, 8))
	((Present, 1000001002))
	((SharedPresent, 1000111000))
)

GANYMEDE_ENUM_NUMBERED(EuropaPipelineBindPoint,
	((Graphics, 0))
	((Compute, 1))
	((RayTracing, 1000165000))
)

GANYMEDE_ENUM_FLAGS(EuropaPipelineStage,
	((TopOfPipe, 0x00000001))
	((DrawIndirect, 0x00000002))
	((VertexInput, 0x00000004))
	((VertexShader, 0x00000008))
	((TessControlShader, 0x00000010))
	((TessEvalShader, 0x00000020))
	((GeometryShader, 0x00000040))
	((FragmentShader, 0x00000080))
	((EarlyFragmentTests, 0x00000100))
	((LateFragmentTests, 0x00000200))
	((ColorAttachmentOutput, 0x00000400))
	((ComputeShader, 0x00000800))
	((Transfer, 0x00001000))
	((BottomOfPipe, 0x00002000))
	((Host, 0x00004000))
	((AllGraphics, 0x00008000))
	((AllCommands, 0x00010000))
)

GANYMEDE_ENUM_FLAGS(EuropaAccess,
	((None, 0x0))
	((IndirectCommandRead, 0x00000001))
	((IndexRead, 0x00000002))
	((VertexAttrRead, 0x00000004))
	((UniformRead, 0x00000008))
	((InputAttachmentRead, 0x00000010))
	((ShaderRead, 0x00000020))
	((ShaderWrite, 0x00000040))
	((ColorAttachmentRead, 0x00000080))
	((ColorAttachmentWrite, 0x00000100))
	((DepthStencilAttachmentRead, 0x00000200))
	((DepthStencilAttachmentWrite, 0x00000400))
	((TransferRead, 0x00000800))
	((TransferWrite, 0x00001000))
	((HostRead, 0x00002000))
	((HostWrite, 0x00004000))
	((MemoryRead, 0x00008000))
	((MemoryWrite, 0x00010000))
)

GANYMEDE_ENUM_FLAGS(EuropaBufferUsage,
	((None, 0x0))
	((TransferSrc, 0x1))
	((TransferDst, 0x2))
	((UniformTexel, 0x4))
	((StorageTexel, 0x8))
	((Uniform, 0x10))
	((Storage, 0x20))
	((Index, 0x40))
	((Vertex, 0x80))
	((Indirect, 0x100))
)

GANYMEDE_ENUM(EuropaMemoryUsage,
	(Unknown)
	(GpuOnly)
	(CpuOnly)
	(Cpu2Gpu)
	(Gpu2Cpu)
	(CpuCopy)
	(GpuLazyAllocated)
)

class EuropaSemaphore
{
public:
	virtual ~EuropaSemaphore() {};
};

class EuropaFence
{
public:
	virtual ~EuropaFence() {};
};

struct EuropaSurfaceCapabilities
{
	uint32 minImageCount;
	uint32 maxImageCount;
	glm::uvec2 currentExtent;
	glm::uvec2 maxExtent;
	uint32 maxArrayLayers;
	EuropaSurfaceTransform currentTransform;
};

struct EuropaSurfaceFormat
{
	EuropaImageFormat format;
	EuropaColorSpace colorSpace;
};

struct EuropaSwapChainCapabilities
{
	EuropaSurfaceCapabilities surfaceCaps;
	std::vector<EuropaSurfaceFormat> formats;
	std::vector<EuropaPresentMode> presentModes;
};

class EuropaSurface
{
public:
	virtual ~EuropaSurface() {};
};

struct EuropaQueueFamilyProperties
{
	glm::u32vec3 minImageTransferGranularity;
	EuropaQueueCapabilities capabilityFlags;
	uint32 queueCount;
	uint32 queueIndex;
	bool presentSupport;
};

class EuropaCmdlist;

struct EuropaSwapChainCreateInfo
{
	EuropaSurface* surface;
	uint32 imageCount;
	EuropaImageFormat format;
	EuropaColorSpace colorSpace;
	glm::uvec2 extent;
	uint32 numLayers;
	EuropaPresentMode presentMode;
	EuropaSurfaceTransform surfaceTransform;
};

class EuropaSwapChain
{
public:
	virtual uint32 AcquireNextImage(EuropaSemaphore* semaphore) = 0;

	virtual ~EuropaSwapChain() {};
};

class EuropaQueue
{
public:
	virtual void Submit(
		uint32 waitSemaphoreCount, EuropaSemaphore** waitSemaphores, EuropaPipelineStage* waitStages,
		uint32 cmdlistCount, EuropaCmdlist** cmdlists,
		uint32 signalSemaphoreCount, EuropaSemaphore** signalSemaphores, EuropaFence* fence = nullptr) = 0;
	virtual void Submit(EuropaCmdlist* cmdlist) = 0;
	virtual void Present(uint32 waitSemaphoreCount, EuropaSemaphore** waitSemaphores, uint32 swapchainCount, EuropaSwapChain** swapchains, uint32 imageIndex) = 0;
	virtual void WaitIdle() = 0;

	virtual ~EuropaQueue() {};
};

class EuropaImage
{
public:
	virtual ~EuropaImage() {};
};

struct EuropaImageViewCreateInfo
{
	EuropaImage* image;
	EuropaImageViewType type;
	EuropaImageFormat format;
	uint32 minMipLevel;
	uint32 numMipLevels;
	uint32 minArrayLayer;
	uint32 numArrayLayers;
};

class EuropaImageView
{
public:
	virtual ~EuropaImageView() {};
};

class EuropaShaderModule
{
public:
	virtual ~EuropaShaderModule() {};
};

struct EuropaShaderStageInfo
{
	EuropaShaderStage stage;
	EuropaShaderModule* module;
	const char* entryPoint;
};

struct EuropaVertexInputBindingInfo
{
	uint32 binding;
	uint32 stride;
	bool perInstance;
};

struct EuropaVertexAttributeBindingInfo
{
	uint32 binding;
	uint32 location;
	uint32 offset;
	EuropaImageFormat format;
};

struct EuropaVertexInputStageInfo
{		
	uint32 vertexBindingCount;
	uint32 attributeBindingCount;
	EuropaVertexInputBindingInfo* vertexBindings;
	EuropaVertexAttributeBindingInfo* attributeBindings;
};

struct EuropaInputAssemblyStageInfo
{
	EuropaPrimitiveTopology topology = EuropaPrimitiveTopology::TriangleList;
	bool primitiveRestartEnable = false;
};

struct EuropaViewport
{
	glm::vec2 position;
	glm::vec2 size;
	float minDepth;
	float maxDepth;
};

struct EuropaScissor
{
	glm::ivec2 position;
	glm::ivec2 size;
};

struct EuropaPipelineViewportStateInfo
{
	uint32 viewportCount;
	uint32 scissorCount;
	EuropaViewport* viewports;
	EuropaScissor* scissors;
};

struct EuropaRasterizerStateInfo
{
	bool depthClamp = false;
	bool rasterizerDiscard = false;
	bool cullBackFace = false;
	bool cullFrontFace = false;
	bool counterClockwise = true;
};

struct EuropaPipelineLayoutInfo
{
	uint32 setLayoutCount;
	uint32 pushConstantRangeCount;
	// FIXME: Support more than 0 on these values
};

class EuropaPipelineLayout
{
public:
	virtual ~EuropaPipelineLayout() {};
};

struct EuropaAttachmentInfo
{
	EuropaImageFormat format;
	EuropaAttachmentLoadOp loadOp;
	EuropaAttachmentStoreOp storeOp;
	EuropaAttachmentLoadOp stencilLoadOp;
	EuropaAttachmentStoreOp stencilStoreOp;
	EuropaImageLayout initialLayout;
	EuropaImageLayout finalLayout;
};

struct EuropaAttachmentReference
{
	uint32 attachment;
	EuropaImageLayout layout;
};

class EuropaRenderPass
{
public:
	static const uint32 SubpassExternal = 0xFFFFFFFF;

	virtual uint32 AddAttachment(EuropaAttachmentInfo& attachment) = 0;
	virtual uint32 AddSubpass(EuropaPipelineBindPoint bindPoint, std::vector<EuropaAttachmentReference>& attachments) = 0;
	virtual void AddDependency(uint32 srcPass, uint32 dstPass, EuropaPipelineStage srcStage, EuropaAccess srcAccess, EuropaPipelineStage dstStage, EuropaAccess dstAccess) = 0;
	virtual void CreateRenderpass() = 0;

	virtual ~EuropaRenderPass() {};
};

struct EuropaBufferInfo
{
	uint32 size;
	EuropaBufferUsage usage;
	EuropaMemoryUsage memoryUsage;
	bool exclusive;
};

class EuropaBuffer
{
public:
	virtual void* MapT() = 0;
	virtual void Unmap() = 0;
	virtual EuropaBufferInfo GetInfo() = 0;
	
	template <typename T> T* Map() { return reinterpret_cast<T*>(this->MapT()); };

	virtual ~EuropaBuffer() {};
};

struct EuropaGraphicsPipelineCreateInfo
{
	uint32 shaderStageCount = 0;
	EuropaShaderStageInfo* stages = nullptr;
	EuropaVertexInputStageInfo vertexInput;
	EuropaInputAssemblyStageInfo inputAssembly;
	EuropaViewport viewport;
	EuropaScissor scissor;
	EuropaRasterizerStateInfo rasterizer;
	// FIXME: Support multi sample
	// FIXME: Support depth sencil
	// FIXME: Support blending
	// FIXME: Support dynamic state
	EuropaPipelineLayout* layout = nullptr;
	EuropaRenderPass* renderpass = nullptr;
	uint32 targetSubpass = 0;
};

class EuropaGraphicsPipeline
{
public:
	virtual ~EuropaGraphicsPipeline() {};
};

struct EuropaFramebufferCreateInfo
{
	EuropaRenderPass* renderpass;
	std::vector<EuropaImageView*> attachments;
	uint32 width;
	uint32 height;
	uint32 layers;
};

class EuropaFramebuffer
{
public:
	virtual ~EuropaFramebuffer() {};
};

class EuropaCmdlist
{
public:
	virtual void Begin() = 0;
	virtual void End() = 0;
	virtual void BeginRenderpass(EuropaRenderPass* renderpass, EuropaFramebuffer* framebuffer, glm::ivec2 offset, glm::uvec2 extent, uint32 clearValueCount, glm::vec4 clearColor) = 0;
	virtual void EndRenderpass() = 0;
	virtual void BindPipeline(EuropaGraphicsPipeline* pipeline) = 0;
	virtual void DrawInstanced(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) = 0;
	virtual void DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, uint32 firstVertex, uint32 firstInstance) = 0;
	virtual void BindVertexBuffer(EuropaBuffer* buffer, uint32 offset, uint32 binding) = 0;
	virtual void BindIndexBuffer(EuropaBuffer* buffer, uint32 offset, EuropaImageFormat indexFormat) = 0;
	virtual void CopyBuffer(EuropaBuffer* dst, EuropaBuffer* src, uint32 size, uint32 srcOffset = 0, uint32 dstOffset = 0) = 0;

	virtual ~EuropaCmdlist() {};
};

class EuropaCommandPool
{
public:
	virtual std::vector<EuropaCmdlist*> AllocateCommandBuffers(uint8 level, uint32 count) = 0;
	virtual ~EuropaCommandPool() {};
};

class EuropaDevice
{
public:
	virtual EuropaDeviceType GetType() = 0;
	virtual std::string GetName() = 0;
	virtual std::vector<EuropaQueueFamilyProperties> GetQueueFamilies(EuropaSurface* surface) = 0;
	virtual void CreateLogicalDevice(uint32 queueFamilyCount, EuropaQueueFamilyProperties* queues, uint32* queueCount) = 0;
	virtual EuropaQueue* GetQueue(EuropaQueueFamilyProperties& queue) = 0;
	virtual EuropaSwapChainCapabilities getSwapChainCapabilities(EuropaSurface* surface) = 0;
	virtual EuropaSwapChain* CreateSwapChain(EuropaSwapChainCreateInfo& args) = 0;
	virtual std::vector<EuropaImage*> GetSwapChainImages(EuropaSwapChain* swapChain) = 0;
	virtual EuropaImageView* CreateImageView(EuropaImageViewCreateInfo& args) = 0;
	virtual EuropaFramebuffer* CreateFramebuffer(EuropaFramebufferCreateInfo& args) = 0;
	virtual EuropaShaderModule* CreateShaderModule(const uint32* spvBinary, uint32 size) = 0;
	virtual EuropaPipelineLayout* CreatePipelineLayout(EuropaPipelineLayoutInfo& args) = 0;
	virtual EuropaRenderPass* CreateRenderPassBuilder() = 0;
	virtual EuropaGraphicsPipeline* CreateGraphicsPipeline(EuropaGraphicsPipelineCreateInfo& args) = 0;
	virtual EuropaCommandPool* CreateCommandPool(EuropaQueueFamilyProperties& queue) = 0;
	virtual EuropaSemaphore* CreateSema() = 0;
	virtual EuropaFence* CreateFence(bool createSignaled = false) = 0;
	virtual void WaitIdle() = 0;
	virtual void WaitForFences(uint32 numFences, EuropaFence** fences, bool waitAll = true, uint64 timeout = UINT64_MAX) = 0;
	virtual void ResetFences(uint32 numFences, EuropaFence** fences) = 0;
	virtual EuropaBuffer* CreateBuffer(EuropaBufferInfo& args) = 0;
	virtual ~EuropaDevice() {};
};

class Europa
{
public:
	virtual ~Europa() {};

	virtual std::vector<EuropaDevice*> GetDevices() = 0;
	virtual EuropaSurface* CreateSurface(IoSurface* ioSurface) = 0;
};
