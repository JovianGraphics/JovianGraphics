#pragma once

#include "Europa.h"

#ifdef EUROPA_VULKAN

#include <vulkan/vulkan.h>

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

	~EuropaDeviceVk();
};

class EuropaQueueVk : public EuropaQueue
{
public:
	VkQueue handle;

	~EuropaQueueVk() {};
};

class EuropaCmdlistVk : public EuropaCmdlist
{
public:
	~EuropaCmdlistVk() {};
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