#include "EuropaVk.h"
#include "Ganymede/Source/Ganymede.h"

#include <vector>
#include <stdexcept>
#include <iostream>

#include <Io/Source/Io.h>

#ifdef IO_WIN32
#include <vulkan/vulkan_win32.h>
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    GanymedePrint "Vulkan Debug:", pCallbackData->pMessage;

    return VK_FALSE;
}

void EuropaVk::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
}

void EuropaVk::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VkResult EuropaVk::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void EuropaVk::SetupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

EuropaVk::EuropaVk()
{
    // Create Info
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Jovian Graphics";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Jovian Graphics";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Enumerate & enable extensions
    std::vector<const char*> enabledExtensions = {};
    {
        uint32 extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        GanymedePrint "Available extensions:";

        for (const auto& extension : extensions) {
            GanymedePrint extension.extensionName;
        }

        enabledExtensions.push_back("VK_KHR_surface");

#ifdef IO_WIN32
        enabledExtensions.push_back("VK_KHR_win32_surface");
#endif
#ifdef _DEBUG
        enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    }

    // Enable validation layers if in Debug mode
    std::vector<const char*> validationLayers = {};
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
#ifdef _DEBUG
    {
        validationLayers.push_back("VK_LAYER_KHRONOS_validation");
    
        uint32 layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                throw std::runtime_error("Vulkan validation layers requested, but not available!");
            }
        }

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
#endif

    createInfo.enabledExtensionCount = uint32(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();
    createInfo.enabledLayerCount = static_cast<uint32>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
    
    // Create Instance
    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create Vulkan instance!");
    }

    SetupDebugMessenger();
}

EuropaVk::~EuropaVk()
{
#ifdef _DEBUG
    DestroyDebugUtilsMessengerEXT(m_instance, debugMessenger, nullptr);
#endif

    vkDestroyInstance(m_instance, nullptr);
}

std::vector<EuropaDevice*> EuropaVk::GetDevices()
{
    std::vector<EuropaDevice*> devices = {};

    uint32 deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> vkPhyDevices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, vkPhyDevices.data());

    for (VkPhysicalDevice& pDevice : vkPhyDevices)
    {
        EuropaDeviceVk* device = new EuropaDeviceVk;
        device->m_phyDevice = pDevice;

        devices.push_back((EuropaDevice*)(device));
    }

    return devices;
}

EuropaSurface* EuropaVk::CreateSurface(IoSurface* _ioSurface)
{
#ifdef IO_WIN32
    IoSurfaceWin32* ioSurface = reinterpret_cast<IoSurfaceWin32*>(_ioSurface);
    EuropaSurfaceVk* surface = new EuropaSurfaceVk();

    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = ioSurface->m_hInstance;
    createInfo.hwnd = ioSurface->m_hwnd;

    vkCreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &surface->m_surface);

    return surface;
#endif

    return nullptr;
}

std::string EuropaDeviceVk::GetName()
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(m_phyDevice, &deviceProperties);

    return std::string(deviceProperties.deviceName);
}

EuropaDeviceType EuropaDeviceVk::GetType()
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(m_phyDevice, &deviceProperties);

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        return EuropaDeviceType::Discrete;
    else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        return EuropaDeviceType::Integrated;
    else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
        return EuropaDeviceType::Virtual;
    else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
        return EuropaDeviceType::CPU;
    else
        return EuropaDeviceType::Virtual;
}

std::vector<EuropaQueueFamilyProperties> EuropaDeviceVk::GetQueueFamilies(EuropaSurface* _surface)
{
    std::vector<EuropaQueueFamilyProperties> families;

    uint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_phyDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_phyDevice, &queueFamilyCount, queueFamilies.data());

    VkSurfaceKHR surface = reinterpret_cast<EuropaSurfaceVk*>(_surface)->m_surface;

    uint32 index = 0;
    for (VkQueueFamilyProperties& prop : queueFamilies)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_phyDevice, index, surface, &presentSupport);

        families.push_back(EuropaQueueFamilyProperties{
            glm::u32vec3(
                prop.minImageTransferGranularity.width,
                prop.minImageTransferGranularity.height,
                prop.minImageTransferGranularity.depth
            ),
            EuropaQueueCapabilities(prop.queueFlags),
            prop.queueCount,
            index,
            presentSupport != 0
        });
        index++;
    }

    return families;
}

void EuropaDeviceVk::CreateLogicalDevice(uint32 queueFamilyCount, EuropaQueueFamilyProperties* queues, uint32* queueCount)
{
    std::vector<VkDeviceQueueCreateInfo>queueCreateInfo;

    static const float priority = 1.0f;

    for (uint32 i = 0; i < queueFamilyCount; i++)
    {
        VkDeviceQueueCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = queues[i].queueIndex;
        info.queueCount = queueCount[i];
        info.pQueuePriorities = &priority;
        queueCreateInfo.push_back(info);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfo.data();
    createInfo.queueCreateInfoCount = queueFamilyCount;

    createInfo.pEnabledFeatures = &deviceFeatures;

    // FIXME: Hard-coded, may want to fix this later to support headless devices / multiple devices
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    createInfo.enabledExtensionCount = static_cast<uint32>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(m_phyDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }
}

EuropaQueue* EuropaDeviceVk::GetQueue(EuropaQueueFamilyProperties& queue)
{
    EuropaQueueVk* vkQueue = new EuropaQueueVk;
    
    vkGetDeviceQueue(m_device, queue.queueIndex, 0, &vkQueue->handle);

    return vkQueue;
}

EuropaDeviceVk::~EuropaDeviceVk()
{
    if (m_device)
        vkDestroyDevice(m_device, nullptr);
}
