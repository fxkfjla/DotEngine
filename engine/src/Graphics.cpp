#include "Graphics.h"

#include <iostream>
#include <cstring>
#include <optional>
#include <set>

Graphics::Graphics(Window& wnd)
    : wnd(wnd)
{
    initVulkan();
    initDebugMessenger();
    initSurface();
    selectPhysicalDevice();
    initLogicalDevice();
}

Graphics::~Graphics()
{
    vkDestroyDevice(device, nullptr);
    destroyDebugMessenger(vkInst, debugMessenger, nullptr);
    vkDestroySurfaceKHR(vkInst, surface, nullptr);;
    vkDestroyInstance(vkInst, nullptr);
}

void Graphics::initVulkan()
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "DotEngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instInfo = {};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo = &appInfo;

// vulkan is platform agnostic, meaning that we need window extension to interface with window system
    const auto&& requiredExtensions = getRequiredExtensions();
    instInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    instInfo.ppEnabledExtensionNames = requiredExtensions.data();

    if(validationLayersEnabled && !validationLayersSupported())
        throw std::runtime_error("Requested validation layers are not supported!");

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo; // outside scope to ensure it doesn't get destroyed before vkCreateInstance
    if(validationLayersEnabled)
    {
        instInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instInfo.ppEnabledLayerNames = validationLayers.data();
    // to be able to debug vkCreateInstance and vkDestroyInstance calls
        setDebugMessengerCreateInfo(debugMessengerInfo);
        instInfo.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugMessengerInfo); 
    }
    else
    {
        instInfo.enabledLayerCount = 0;
        instInfo.pNext = nullptr;
    }

    if(vkCreateInstance(&instInfo, nullptr, &vkInst) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the vk instance!");
}

void Graphics::initDebugMessenger()
{
    if(!validationLayersEnabled)
        return;
    
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo;
    setDebugMessengerCreateInfo(debugMessengerInfo);

    if(createDebugMessenger(vkInst, &debugMessengerInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        throw std::runtime_error("Failed to create a debug messenger!");
}

void Graphics::initSurface()
{
    if(glfwCreateWindowSurface(vkInst, wnd, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface!");
}

void Graphics::selectPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInst, &deviceCount, nullptr);

    if(deviceCount == 0)
        throw std::runtime_error("Failed to find any GPU!");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInst, &deviceCount, devices.data());

    for(const auto& device : devices)
    {
        if(deviceIsSupported(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if(physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("GPU not supported!");
}

void Graphics::initLogicalDevice()
{
    std::set<uint32_t> uniqueQueues = {queueIndices.graphicFamily.value(), queueIndices.presentFamily.value()};
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;
    for(auto queue : uniqueQueues)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queue;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.emplace_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(uniqueQueues.size());
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if(validationLayersEnabled)
    {
        deviceCreateInfo.enabledLayerCount = validationLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
        deviceCreateInfo.enabledLayerCount = 0;

    if(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device!");
    
// 0 because we are creating only 1 queue
    vkGetDeviceQueue(device, queueIndices.graphicFamily.value(), 0, &graphicQueue);
    vkGetDeviceQueue(device, queueIndices.presentFamily.value(), 0, &presentQueue);
}

std::vector<const char*> Graphics::getRequiredExtensions() const noexcept
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if(validationLayersEnabled)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

bool Graphics::deviceIsSupported(const VkPhysicalDevice& device) noexcept
{
// basic device properties like name, type, vulkan version
    // VkPhysicalDeviceProperties deviceProperties;
    // vkGetPhysicalDeviceProperties(device, &deviceProperties);
// optional features like texture compression, 64 bit floats, multi viewport rendering
    // VkPhysicalDeviceFeatures deviceFeatures;
    // vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // return 
    //     deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
    //     deviceFeatures.geometryShader; && has_value

    setQueueFamiliesOf(device);

    return queueIndices.found() && deviceExtensionsSupported(device);
}

bool Graphics::deviceExtensionsSupported(const VkPhysicalDevice& device) const noexcept
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for(const auto& extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

void Graphics::setQueueFamiliesOf(const VkPhysicalDevice& device) noexcept
{
    uint32_t queueFamiliesCounter = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCounter, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCounter);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCounter, queueFamilies.data());

    // searching of queues indices that support selected operations

    VkBool32 presentSupport = VK_FALSE;
    uint32_t i = 0;
    for(const auto& queueFamily : queueFamilies)
    {
        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueIndices.graphicFamily = i;

        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if(presentSupport)
            queueIndices.presentFamily = i;

        if(queueIndices.found())
            break;

        i++;
    }
}

bool Graphics::validationLayersSupported() const noexcept
{
    uint32_t validationLayersCount = 0;
    vkEnumerateInstanceLayerProperties(&validationLayersCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(validationLayersCount);
    vkEnumerateInstanceLayerProperties(&validationLayersCount, availableLayers.data());

    std::set<std::string> requiredLayers(validationLayers.begin(), validationLayers.end());

    for(const auto& layer : availableLayers)
        requiredLayers.erase(layer.layerName);

    return requiredLayers.empty();
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
)
{
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

VkResult Graphics::createDebugMessenger
(
    VkInstance vkInst,
    const VkDebugUtilsMessengerCreateInfoEXT* pDebugMessengerInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
) noexcept
{
    auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInst, "vkCreateDebugUtilsMessengerEXT");
// because this function is an extension function it is not loaded, and we need to look it up ourselves
    if(fn != nullptr)
        return fn(vkInst, pDebugMessengerInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Graphics::destroyDebugMessenger
(
    VkInstance vkInst,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
) noexcept
{
    if(validationLayersEnabled)
    {
        auto fn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInst, "vkDestroyDebugUtilsMessengerEXT");

        if(fn != nullptr)
            fn(vkInst, debugMessenger, pAllocator);
    }
}

void Graphics::setDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessengerInfo) noexcept
{
    debugMessengerInfo = {};
    debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

    debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    debugMessengerInfo.pfnUserCallback = debugCallback;
    debugMessengerInfo.pUserData = nullptr;
}