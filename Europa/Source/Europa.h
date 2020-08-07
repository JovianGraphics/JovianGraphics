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

class EuropaQueue
{
public:
	virtual ~EuropaQueue() {};
};

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
	virtual ~EuropaSwapChain() {};
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
	virtual ~EuropaDevice() {};
};

class EuropaCmdlist
{
public:
	virtual ~EuropaCmdlist() {};
};

class Europa
{
public:
	virtual ~Europa() {};

	virtual std::vector<EuropaDevice*> GetDevices() = 0;
	virtual EuropaSurface* CreateSurface(IoSurface* ioSurface) = 0;
};
