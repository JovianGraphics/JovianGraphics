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

	~EuropaDeviceVk();
};

class EuropaSwapChainVk : public EuropaSwapChain
{
public:
	EuropaDeviceVk* m_device;
	VkSwapchainKHR m_swapchain;

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