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

EuropaSwapChainCapabilities EuropaDeviceVk::getSwapChainCapabilities(EuropaSurface* _surface)
{
    EuropaSwapChainCapabilities caps;

    EuropaSurfaceVk* surface = static_cast<EuropaSurfaceVk*>(_surface);
    VkSurfaceCapabilitiesKHR vkCaps;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_phyDevice, surface->m_surface, &vkCaps);
    caps.surfaceCaps.currentExtent = glm::uvec2(vkCaps.currentExtent.width, vkCaps.currentExtent.height);
    caps.surfaceCaps.maxArrayLayers = vkCaps.maxImageArrayLayers;
    caps.surfaceCaps.maxExtent = glm::uvec2(vkCaps.maxImageExtent.width, vkCaps.maxImageExtent.height);
    caps.surfaceCaps.maxImageCount = vkCaps.maxImageCount;
    caps.surfaceCaps.minImageCount = vkCaps.minImageCount;
    caps.surfaceCaps.currentTransform = EuropaSurfaceTransform(vkCaps.currentTransform);

    uint32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_phyDevice, surface->m_surface, &formatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> formats(formatCount);

    if (formatCount != 0) {
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_phyDevice, surface->m_surface, &formatCount, formats.data());
    }

    for (VkSurfaceFormatKHR& format : formats)
    {
        caps.formats.push_back({
            VkFormat2EuropaImageFormat(format.format),
            EuropaColorSpace(format.colorSpace)
        });
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_phyDevice, surface->m_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        caps.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_phyDevice, surface->m_surface, &presentModeCount, (VkPresentModeKHR*)caps.presentModes.data());
    }

    return caps;
}

EuropaSwapChain* EuropaDeviceVk::CreateSwapChain(EuropaSwapChainCreateInfo& args)
{
    EuropaSwapChainVk* swapChain = new EuropaSwapChainVk();
    EuropaSurfaceVk* surface = static_cast<EuropaSurfaceVk*>(args.surface);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface->m_surface;
    createInfo.minImageCount = args.imageCount;
    createInfo.imageFormat = EuropaImageFormat2VkFormat(args.format);
    createInfo.imageColorSpace = EuropaColorSpace2VkColorSpaceKHR(args.colorSpace);
    createInfo.imageExtent = VkExtent2D{ args.extent.x, args.extent.y };
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = VkSurfaceTransformFlagBitsKHR(args.surfaceTransform);
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VkPresentModeKHR(args.presentMode);
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // FIXME: Support concurrent sharing (present and graphics are on different queue)
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;

    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &swapChain->m_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    swapChain->m_device = this;

    return swapChain;
}

EuropaDeviceVk::~EuropaDeviceVk()
{
    if (m_device)
        vkDestroyDevice(m_device, nullptr);
}

EuropaSwapChainVk::~EuropaSwapChainVk()
{
    if (m_swapchain)
        vkDestroySwapchainKHR(m_device->m_device, m_swapchain, nullptr);
}

VkFormat formatsFromEuropa[] = {
    VK_FORMAT_UNDEFINED, // Undefined

    VK_FORMAT_R4G4_UNORM_PACK8, // RG4Unorm
    VK_FORMAT_R4G4B4A4_UNORM_PACK16, // RGBA4Unorm

    VK_FORMAT_R5G6B5_UNORM_PACK16, // R5G6B5Unorm
    VK_FORMAT_R5G5B5A1_UNORM_PACK16, // RGB5A1Unorm

    VK_FORMAT_R8_UNORM, // R8Unorm
    VK_FORMAT_R8_SNORM, // R8Snorm
    VK_FORMAT_R8_UINT, // R8UI
    VK_FORMAT_R8_SINT, // R8I
    VK_FORMAT_R8_SRGB, // R8sRGB
    VK_FORMAT_R8G8_UNORM, // RG8Unorm
    VK_FORMAT_R8G8_SNORM, // RG8Snorm
    VK_FORMAT_R8G8_UINT, // RG8UI
    VK_FORMAT_R8G8_SINT, // RG8I
    VK_FORMAT_R8G8_SRGB, // RG8sRGB
    VK_FORMAT_R8G8B8_UNORM, // RGB8Unorm
    VK_FORMAT_R8G8B8_SNORM, // RGB8Snorm
    VK_FORMAT_R8G8B8_UINT, // RGB8UI
    VK_FORMAT_R8G8B8_SINT, // RGB8I
    VK_FORMAT_R8G8B8_SRGB, // RGB8sRGB
    VK_FORMAT_R8G8B8A8_UNORM, // RGBA8Unorm
    VK_FORMAT_R8G8B8A8_SNORM, // RGBA8Snorm
    VK_FORMAT_R8G8B8A8_UINT, // RGBA8UI
    VK_FORMAT_R8G8B8A8_SINT, // RGBA8I
    VK_FORMAT_R8G8B8A8_SRGB, // RGBA8sRGB

    VK_FORMAT_B8G8R8_UNORM, // BGR8Unorm
    VK_FORMAT_B8G8R8_SNORM, // BGR8Snorm
    VK_FORMAT_B8G8R8_UINT, // BGR8UI
    VK_FORMAT_B8G8R8_SINT, // BGR8I
    VK_FORMAT_B8G8R8_SRGB, // BGR8sRGB
    VK_FORMAT_B8G8R8A8_UNORM, // BGRA8Unorm
    VK_FORMAT_B8G8R8A8_SNORM, // BGRA8Snorm
    VK_FORMAT_B8G8R8A8_UINT, // BGRA8UI
    VK_FORMAT_B8G8R8A8_SINT, // BGRA8I
    VK_FORMAT_B8G8R8A8_SRGB, // BGRA8sRGB

    VK_FORMAT_R16_UNORM, // R16Unorm
    VK_FORMAT_R16_SNORM, // R16Snorm
    VK_FORMAT_R16_UINT, // R16UI
    VK_FORMAT_R16_SINT, // R16I
    VK_FORMAT_R16_SFLOAT, // R16F
    VK_FORMAT_R16G16_UNORM, // RG16Unorm
    VK_FORMAT_R16G16_SNORM, // RG16Snorm
    VK_FORMAT_R16G16_UINT, // RG16UI
    VK_FORMAT_R16G16_SINT, // RG16I
    VK_FORMAT_R16G16_SFLOAT, // RG16F
    VK_FORMAT_R16G16B16_UNORM, // RGB16Unorm
    VK_FORMAT_R16G16B16_SNORM, // RGB16Snorm
    VK_FORMAT_R16G16B16_UINT, // RGB16UI
    VK_FORMAT_R16G16B16_SINT, // RGB16I
    VK_FORMAT_R16G16B16_SFLOAT, // RGB16F
    VK_FORMAT_R16G16B16A16_UNORM, // RGBA16Unorm
    VK_FORMAT_R16G16B16A16_SNORM, // RGBA16Snorm
    VK_FORMAT_R16G16B16A16_UINT, // RGBA16UI
    VK_FORMAT_R16G16B16A16_SINT, // RGBA16I
    VK_FORMAT_R16G16B16A16_SFLOAT, // RGBA16F

    VK_FORMAT_R32_UINT, // R32UI
    VK_FORMAT_R32_SINT, // R32I
    VK_FORMAT_R32_SFLOAT, // R32F
    VK_FORMAT_R32G32_UINT, // RG32UI
    VK_FORMAT_R32G32_SINT, // RG32I
    VK_FORMAT_R32G32_SFLOAT, // RG32F
    VK_FORMAT_R32G32B32_UINT, // RGB32UI
    VK_FORMAT_R32G32B32_SINT, // RGB32I
    VK_FORMAT_R32G32B32_SFLOAT, // RGB32F
    VK_FORMAT_R32G32B32A32_UINT, // RGBA32UI
    VK_FORMAT_R32G32B32A32_SINT, // RGBA32I
    VK_FORMAT_R32G32B32A32_SFLOAT, // RGBA32F

    VK_FORMAT_B10G11R11_UFLOAT_PACK32, // B10G11R11UFloat
    VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, // E5B9G9R9UFloat

    VK_FORMAT_BC1_RGB_UNORM_BLOCK, // BC1RGBUnorm
    VK_FORMAT_BC1_RGB_SRGB_BLOCK, // BC1RGBsRGB
    VK_FORMAT_BC1_RGBA_UNORM_BLOCK, // BC1RGBAUnorm
    VK_FORMAT_BC1_RGBA_SRGB_BLOCK, // BC1RGBAsRGB

    VK_FORMAT_BC2_UNORM_BLOCK, // BC2RGBAUnorm
    VK_FORMAT_BC2_SRGB_BLOCK, // BC2RGBAsRGB

    VK_FORMAT_BC3_UNORM_BLOCK, // BC3RGBAUnorm
    VK_FORMAT_BC3_SRGB_BLOCK, // BC3RGBAsRGB

    VK_FORMAT_BC4_UNORM_BLOCK, // BC4RGBAUnorm
    VK_FORMAT_BC4_SNORM_BLOCK, // BC4RGBASnorm

    VK_FORMAT_BC5_UNORM_BLOCK, // BC5RGBAUnorm
    VK_FORMAT_BC5_SNORM_BLOCK, // BC5RGBASnorm
};

EuropaImageFormat formatsFromVk[] = {
    EuropaImageFormat::Undefined, // VK_FORMAT_UNDEFINED = 0,
    EuropaImageFormat::RG4Unorm, // VK_FORMAT_R4G4_UNORM_PACK8 = 1,
    EuropaImageFormat::RGBA4Unorm, // VK_FORMAT_R4G4B4A4_UNORM_PACK16 = 2,
    EuropaImageFormat::Undefined, // VK_FORMAT_B4G4R4A4_UNORM_PACK16 = 3,
    EuropaImageFormat::R5G6B5Unorm, // VK_FORMAT_R5G6B5_UNORM_PACK16 = 4,
    EuropaImageFormat::Undefined, // VK_FORMAT_B5G6R5_UNORM_PACK16 = 5,
    EuropaImageFormat::RGB5A1Unorm, // VK_FORMAT_R5G5B5A1_UNORM_PACK16 = 6,
    EuropaImageFormat::Undefined, // VK_FORMAT_B5G5R5A1_UNORM_PACK16 = 7,
    EuropaImageFormat::Undefined, // VK_FORMAT_A1R5G5B5_UNORM_PACK16 = 8,
    EuropaImageFormat::R8Unorm, // VK_FORMAT_R8_UNORM = 9,
    EuropaImageFormat::R8Snorm, // VK_FORMAT_R8_SNORM = 10,
    EuropaImageFormat::Undefined, // VK_FORMAT_R8_USCALED = 11,
    EuropaImageFormat::Undefined, // VK_FORMAT_R8_SSCALED = 12,
    EuropaImageFormat::R8UI, // VK_FORMAT_R8_UINT = 13,
    EuropaImageFormat::R8I, // VK_FORMAT_R8_SINT = 14,
    EuropaImageFormat::R8sRGB, // VK_FORMAT_R8_SRGB = 15,
    EuropaImageFormat::RG8Unorm, // VK_FORMAT_R8G8_UNORM = 16,
    EuropaImageFormat::RG8Snorm, // VK_FORMAT_R8G8_SNORM = 17,
    EuropaImageFormat::Undefined, // VK_FORMAT_R8G8_USCALED = 18,
    EuropaImageFormat::Undefined, // VK_FORMAT_R8G8_SSCALED = 19,
    EuropaImageFormat::RG8UI, // VK_FORMAT_R8G8_UINT = 20,
    EuropaImageFormat::RG8I, // VK_FORMAT_R8G8_SINT = 21,
    EuropaImageFormat::RG8sRGB, // VK_FORMAT_R8G8_SRGB = 22,
    EuropaImageFormat::RGB8Unorm, // VK_FORMAT_R8G8B8_UNORM = 23,
    EuropaImageFormat::RGB8Snorm, // VK_FORMAT_R8G8B8_SNORM = 24,
    EuropaImageFormat::Undefined, // VK_FORMAT_R8G8B8_USCALED = 25,
    EuropaImageFormat::Undefined, // VK_FORMAT_R8G8B8_SSCALED = 26,
    EuropaImageFormat::RGB8UI, // VK_FORMAT_R8G8B8_UINT = 27,
    EuropaImageFormat::RGB8I, // VK_FORMAT_R8G8B8_SINT = 28,
    EuropaImageFormat::RGB8sRGB, // VK_FORMAT_R8G8B8_SRGB = 29,
    EuropaImageFormat::BGR8Unorm, // VK_FORMAT_B8G8R8_UNORM = 30,
    EuropaImageFormat::RGB8Snorm, // VK_FORMAT_B8G8R8_SNORM = 31,
    EuropaImageFormat::Undefined, // VK_FORMAT_B8G8R8_USCALED = 32,
    EuropaImageFormat::Undefined, // VK_FORMAT_B8G8R8_SSCALED = 33,
    EuropaImageFormat::BGR8UI, // VK_FORMAT_B8G8R8_UINT = 34,
    EuropaImageFormat::BGR8I, // VK_FORMAT_B8G8R8_SINT = 35,
    EuropaImageFormat::BGR8sRGB, // VK_FORMAT_B8G8R8_SRGB = 36,
    EuropaImageFormat::RGBA8Unorm, // VK_FORMAT_R8G8B8A8_UNORM = 37,
    EuropaImageFormat::RGBA8Snorm, // VK_FORMAT_R8G8B8A8_SNORM = 38,
    EuropaImageFormat::Undefined, // VK_FORMAT_R8G8B8A8_USCALED = 39,
    EuropaImageFormat::Undefined, // VK_FORMAT_R8G8B8A8_SSCALED = 40,
    EuropaImageFormat::RGBA8UI, // VK_FORMAT_R8G8B8A8_UINT = 41,
    EuropaImageFormat::RGBA8I, // VK_FORMAT_R8G8B8A8_SINT = 42,
    EuropaImageFormat::RGBA8sRGB, // VK_FORMAT_R8G8B8A8_SRGB = 43,
    EuropaImageFormat::BGRA8Unorm, // VK_FORMAT_B8G8R8A8_UNORM = 44,
    EuropaImageFormat::BGRA8Snorm, // VK_FORMAT_B8G8R8A8_SNORM = 45,
    EuropaImageFormat::Undefined, // VK_FORMAT_B8G8R8A8_USCALED = 46,
    EuropaImageFormat::Undefined, // VK_FORMAT_B8G8R8A8_SSCALED = 47,
    EuropaImageFormat::BGRA8UI, // VK_FORMAT_B8G8R8A8_UINT = 48,
    EuropaImageFormat::BGRA8I, // VK_FORMAT_B8G8R8A8_SINT = 49,
    EuropaImageFormat::BGRA8sRGB, // VK_FORMAT_B8G8R8A8_SRGB = 50,
    EuropaImageFormat::Undefined, // VK_FORMAT_A8B8G8R8_UNORM_PACK32 = 51,
    EuropaImageFormat::Undefined, // VK_FORMAT_A8B8G8R8_SNORM_PACK32 = 52,
    EuropaImageFormat::Undefined, // VK_FORMAT_A8B8G8R8_USCALED_PACK32 = 53,
    EuropaImageFormat::Undefined, // VK_FORMAT_A8B8G8R8_SSCALED_PACK32 = 54,
    EuropaImageFormat::Undefined, // VK_FORMAT_A8B8G8R8_UINT_PACK32 = 55,
    EuropaImageFormat::Undefined, // VK_FORMAT_A8B8G8R8_SINT_PACK32 = 56,
    EuropaImageFormat::Undefined, // VK_FORMAT_A8B8G8R8_SRGB_PACK32 = 57,
    EuropaImageFormat::Undefined, // VK_FORMAT_A2R10G10B10_UNORM_PACK32 = 58,
    EuropaImageFormat::Undefined, // VK_FORMAT_A2R10G10B10_SNORM_PACK32 = 59,
    EuropaImageFormat::Undefined, // VK_FORMAT_A2R10G10B10_USCALED_PACK32 = 60,
    EuropaImageFormat::Undefined, // VK_FORMAT_A2R10G10B10_SSCALED_PACK32 = 61,
    EuropaImageFormat::Undefined, // VK_FORMAT_A2R10G10B10_UINT_PACK32 = 62,
    EuropaImageFormat::Undefined, // VK_FORMAT_A2R10G10B10_SINT_PACK32 = 63,
    EuropaImageFormat::Undefined, // VK_FORMAT_A2B10G10R10_UNORM_PACK32 = 64,
    EuropaImageFormat::Undefined, // VK_FORMAT_A2B10G10R10_SNORM_PACK32 = 65,
    EuropaImageFormat::Undefined, // VK_FORMAT_A2B10G10R10_USCALED_PACK32 = 66,
    EuropaImageFormat::Undefined, // VK_FORMAT_A2B10G10R10_SSCALED_PACK32 = 67,
    EuropaImageFormat::Undefined, // VK_FORMAT_A2B10G10R10_UINT_PACK32 = 68,
    EuropaImageFormat::Undefined, // VK_FORMAT_A2B10G10R10_SINT_PACK32 = 69,
    EuropaImageFormat::R16Unorm, // VK_FORMAT_R16_UNORM = 70,
    EuropaImageFormat::R16Snorm, // VK_FORMAT_R16_SNORM = 71,
    EuropaImageFormat::Undefined, // VK_FORMAT_R16_USCALED = 72,
    EuropaImageFormat::Undefined, // VK_FORMAT_R16_SSCALED = 73,
    EuropaImageFormat::R16UI, // VK_FORMAT_R16_UINT = 74,
    EuropaImageFormat::R16I, // VK_FORMAT_R16_SINT = 75,
    EuropaImageFormat::R16F, // VK_FORMAT_R16_SFLOAT = 76,
    EuropaImageFormat::RG16Unorm, // VK_FORMAT_R16G16_UNORM = 77,
    EuropaImageFormat::RG16Snorm, // VK_FORMAT_R16G16_SNORM = 78,
    EuropaImageFormat::Undefined, // VK_FORMAT_R16G16_USCALED = 79,
    EuropaImageFormat::Undefined, // VK_FORMAT_R16G16_SSCALED = 80,
    EuropaImageFormat::RG16UI, // VK_FORMAT_R16G16_UINT = 81,
    EuropaImageFormat::RG16I, // VK_FORMAT_R16G16_SINT = 82,
    EuropaImageFormat::RG16F, // VK_FORMAT_R16G16_SFLOAT = 83,
    EuropaImageFormat::RGB16Unorm, // VK_FORMAT_R16G16B16_UNORM = 84,
    EuropaImageFormat::RGB16Snorm, // VK_FORMAT_R16G16B16_SNORM = 85,
    EuropaImageFormat::Undefined, // VK_FORMAT_R16G16B16_USCALED = 86,
    EuropaImageFormat::Undefined, // VK_FORMAT_R16G16B16_SSCALED = 87,
    EuropaImageFormat::RGB16UI, // VK_FORMAT_R16G16B16_UINT = 88,
    EuropaImageFormat::RGB16I, // VK_FORMAT_R16G16B16_SINT = 89,
    EuropaImageFormat::RGB16F, // VK_FORMAT_R16G16B16_SFLOAT = 90,
    EuropaImageFormat::RGBA16Unorm, // VK_FORMAT_R16G16B16A16_UNORM = 91,
    EuropaImageFormat::RGBA16Snorm, // VK_FORMAT_R16G16B16A16_SNORM = 92,
    EuropaImageFormat::Undefined, // VK_FORMAT_R16G16B16A16_USCALED = 93,
    EuropaImageFormat::Undefined, // VK_FORMAT_R16G16B16A16_SSCALED = 94,
    EuropaImageFormat::RGBA16UI, // VK_FORMAT_R16G16B16A16_UINT = 95,
    EuropaImageFormat::RGBA16I, // VK_FORMAT_R16G16B16A16_SINT = 96,
    EuropaImageFormat::RGBA16F, // VK_FORMAT_R16G16B16A16_SFLOAT = 97,
    EuropaImageFormat::R32UI, // VK_FORMAT_R32_UINT = 98,
    EuropaImageFormat::R32I, // VK_FORMAT_R32_SINT = 99,
    EuropaImageFormat::R32F, // VK_FORMAT_R32_SFLOAT = 100,
    EuropaImageFormat::RG32UI, // VK_FORMAT_R32G32_UINT = 101,
    EuropaImageFormat::RG32I, // VK_FORMAT_R32G32_SINT = 102,
    EuropaImageFormat::RG32F, // VK_FORMAT_R32G32_SFLOAT = 103,
    EuropaImageFormat::RGB32UI, // VK_FORMAT_R32G32B32_UINT = 104,
    EuropaImageFormat::RGB32I, // VK_FORMAT_R32G32B32_SINT = 105,
    EuropaImageFormat::RGB32F, // VK_FORMAT_R32G32B32_SFLOAT = 106,
    EuropaImageFormat::RGBA32UI, // VK_FORMAT_R32G32B32A32_UINT = 107,
    EuropaImageFormat::RGBA32I, // VK_FORMAT_R32G32B32A32_SINT = 108,
    EuropaImageFormat::RGBA32F, // VK_FORMAT_R32G32B32A32_SFLOAT = 109,
    EuropaImageFormat::Undefined, // VK_FORMAT_R64_UINT = 110,
    EuropaImageFormat::Undefined, // VK_FORMAT_R64_SINT = 111,
    EuropaImageFormat::Undefined, // VK_FORMAT_R64_SFLOAT = 112,
    EuropaImageFormat::Undefined, // VK_FORMAT_R64G64_UINT = 113,
    EuropaImageFormat::Undefined, // VK_FORMAT_R64G64_SINT = 114,
    EuropaImageFormat::Undefined, // VK_FORMAT_R64G64_SFLOAT = 115,
    EuropaImageFormat::Undefined, // VK_FORMAT_R64G64B64_UINT = 116,
    EuropaImageFormat::Undefined, // VK_FORMAT_R64G64B64_SINT = 117,
    EuropaImageFormat::Undefined, // VK_FORMAT_R64G64B64_SFLOAT = 118,
    EuropaImageFormat::Undefined, // VK_FORMAT_R64G64B64A64_UINT = 119,
    EuropaImageFormat::Undefined, // VK_FORMAT_R64G64B64A64_SINT = 120,
    EuropaImageFormat::Undefined, // VK_FORMAT_R64G64B64A64_SFLOAT = 121,
    EuropaImageFormat::B10G11R11UFloat, // VK_FORMAT_B10G11R11_UFLOAT_PACK32 = 122,
    EuropaImageFormat::E5B9G9R9UFloat, // VK_FORMAT_E5B9G9R9_UFLOAT_PACK32 = 123,
    EuropaImageFormat::Undefined, // VK_FORMAT_D16_UNORM = 124,
    EuropaImageFormat::Undefined, // VK_FORMAT_X8_D24_UNORM_PACK32 = 125,
    EuropaImageFormat::Undefined, // VK_FORMAT_D32_SFLOAT = 126,
    EuropaImageFormat::Undefined, // VK_FORMAT_S8_UINT = 127,
    EuropaImageFormat::Undefined, // VK_FORMAT_D16_UNORM_S8_UINT = 128,
    EuropaImageFormat::Undefined, // VK_FORMAT_D24_UNORM_S8_UINT = 129,
    EuropaImageFormat::Undefined, // VK_FORMAT_D32_SFLOAT_S8_UINT = 130,
    EuropaImageFormat::BC1RGBUnorm, // VK_FORMAT_BC1_RGB_UNORM_BLOCK = 131,
    EuropaImageFormat::BC1RGBsRGB, // VK_FORMAT_BC1_RGB_SRGB_BLOCK = 132,
    EuropaImageFormat::BC1RGBAUnorm, // VK_FORMAT_BC1_RGBA_UNORM_BLOCK = 133,
    EuropaImageFormat::BC1RGBAsRGB, // VK_FORMAT_BC1_RGBA_SRGB_BLOCK = 134,
    EuropaImageFormat::BC2RGBAUnorm, // VK_FORMAT_BC2_UNORM_BLOCK = 135,
    EuropaImageFormat::BC2RGBAsRGB, // VK_FORMAT_BC2_SRGB_BLOCK = 136,
    EuropaImageFormat::BC3RGBAUnorm, // VK_FORMAT_BC3_UNORM_BLOCK = 137,
    EuropaImageFormat::BC3RGBAsRGB, // VK_FORMAT_BC3_SRGB_BLOCK = 138,
    EuropaImageFormat::BC4RGBAUnorm, // VK_FORMAT_BC4_UNORM_BLOCK = 139,
    EuropaImageFormat::BC4RGBASnorm, // VK_FORMAT_BC4_SNORM_BLOCK = 140,
    EuropaImageFormat::BC5RGBAUnorm, // VK_FORMAT_BC5_UNORM_BLOCK = 141,
    EuropaImageFormat::BC5RGBASnorm, // VK_FORMAT_BC5_SNORM_BLOCK = 142,
    EuropaImageFormat::Undefined, // VK_FORMAT_BC6H_UFLOAT_BLOCK = 143,
    EuropaImageFormat::Undefined, // VK_FORMAT_BC6H_SFLOAT_BLOCK = 144,
    EuropaImageFormat::Undefined, // VK_FORMAT_BC7_UNORM_BLOCK = 145,
    EuropaImageFormat::Undefined, // VK_FORMAT_BC7_SRGB_BLOCK = 146,
};

VkFormat EuropaImageFormat2VkFormat(EuropaImageFormat f)
{
    return formatsFromEuropa[uint32(f)];
}

EuropaImageFormat VkFormat2EuropaImageFormat(VkFormat f)
{
    return formatsFromVk[uint32(f)];
}
