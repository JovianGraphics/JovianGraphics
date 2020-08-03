#pragma once

#include "config.h"
#include "Ganymede/Source/Ganymede.h"

#include <vector>
#include <optional>

#include <glm/glm.hpp>

class IoSurface;

enum EuropaDeviceType : int32
{
	Discrete,
	Integrated,
	Virtual,
	CPU,
	Other
};

static const char* EuropaDeviceTypeNames[] = {
	"Discrete",
	"Integrated",
	"Virtual",
	"CPU",
	"Other"
};

enum EuropaQueueCapabilities : uint32
{
	Graphics = 0x00000001,
	Compute = 0x00000002,
	Transfer = 0x00000004,
	SparseBinding = 0x00000008,
	Protected = 0x00000010
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

class EuropaDevice
{
public:
	virtual EuropaDeviceType GetType() = 0;
	virtual std::string GetName() = 0;
	virtual std::vector<EuropaQueueFamilyProperties> GetQueueFamilies(EuropaSurface* surface) = 0;
	virtual void CreateLogicalDevice(uint32 queueFamilyCount, EuropaQueueFamilyProperties* queues, uint32* queueCount) = 0;
	virtual EuropaQueue* GetQueue(EuropaQueueFamilyProperties& queue) = 0;
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
