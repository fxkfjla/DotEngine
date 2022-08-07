#include "Graphics.h"

#include <iostream>
#include <cstring>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>

Graphics::Graphics(Window& wnd)
    : wnd(wnd)
{
    initVulkan();

    if(validationLayersEnabled)
        initDebugMessenger();

    initSurface();
    selectPhysicalDevice();
    initLogicalDevice();
    initSwapChain();

    initImageViews();
}

Graphics::~Graphics()
{
    for(const auto& imageView : swapChainImageViews)
        device.destroyImageView(imageView);

    device.destroySwapchainKHR(swapChain);
    device.destroy();
    vkInst.destroySurfaceKHR(surface);

    if(validationLayersEnabled)
        vkInst.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);

    vkInst.destroy();
}

void Graphics::initVulkan()
{
    vk::ApplicationInfo appInfo = {};
    appInfo.sType = vk::StructureType::eApplicationInfo;
    appInfo.pApplicationName = "DotEngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    vk::InstanceCreateInfo instInfo = {};
    instInfo.sType = vk::StructureType::eInstanceCreateInfo;
    instInfo.pApplicationInfo = &appInfo;

// vulkan is platform agnostic, meaning that we need window extension to interface with window system
    const auto&& requiredExtensions = getRequiredExtensions();
    instInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    instInfo.ppEnabledExtensionNames = requiredExtensions.data();

    if(validationLayersEnabled && !validationLayersSupported())
        throw std::runtime_error("Requested validation layers are not supported!");

    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo; // outside scope to ensure it doesn't get destroyed before vkCreateInstance
    if(validationLayersEnabled)
    {
        instInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instInfo.ppEnabledLayerNames = validationLayers.data();
    // to be able to debug vkCreateInstance and vkDestroyInstance calls
        setDebugMessengerCreateInfo(debugMessengerInfo);
        instInfo.pNext = reinterpret_cast<vk::DebugUtilsMessengerCreateInfoEXT*>(&debugMessengerInfo); 
    }
    else
    {
        instInfo.enabledLayerCount = 0;
        instInfo.pNext = nullptr;
    }

    try
    {
        vkInst = vk::createInstance(instInfo);
        dldi = vk::DispatchLoaderDynamic(vkInst, vkGetInstanceProcAddr);
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << e.what() << '\n';
        throw std::runtime_error("Failed to create the vk instance!");
    }

    if(validationLayersEnabled)
        displayExtensionsInfo(requiredExtensions);
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

bool Graphics::validationLayersSupported() const noexcept
{
    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

    std::set<std::string> requiredLayers(validationLayers.begin(), validationLayers.end());

    for(const auto& layer : availableLayers)
        requiredLayers.erase(layer.layerName);

    return requiredLayers.empty();
}

void Graphics::displayExtensionsInfo(std::vector<const char*> extensions) const noexcept
{
    std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();

    std::cout << "\nViable extensions:\n";
    for(const auto& supportedExtension : supportedExtensions)
        std::cout << '\t' << supportedExtension.extensionName << '\n';
    
    std::cout << "\nSupported extensions:\n";
    for(const auto& extension : extensions)
    {
        bool found = false;
        for(const auto& supportedExtension : supportedExtensions)
            if(strcmp(extension, supportedExtension.extensionName) == 0)
            {
                std::cout << '\t' << extension << " is supported!\n";
                found = true;
                break;
            }

        if(!found)
            std::cout << '\t' << extension << " is not supported!\n";
    }
}

void Graphics::initDebugMessenger() noexcept
{
    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo;
    setDebugMessengerCreateInfo(debugMessengerInfo);

    debugMessenger = vkInst.createDebugUtilsMessengerEXT(debugMessengerInfo, nullptr, dldi);
}

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
)
{
    std::cerr << "Validation layer: " << pCallbackData->pMessage << '\n';

    return VK_FALSE;
}

void Graphics::setDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& debugMessengerInfo) const noexcept
{
    debugMessengerInfo.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;

    debugMessengerInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

    debugMessengerInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

    debugMessengerInfo.pfnUserCallback = debugCallback;
    debugMessengerInfo.pUserData = nullptr;
}

void Graphics::initSurface()
{
    VkSurfaceKHR vkSurface;

    if(glfwCreateWindowSurface(vkInst, wnd, nullptr, &vkSurface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface!");

    surface = vk::SurfaceKHR(vkSurface);
}

void Graphics::selectPhysicalDevice()
{
    std::vector<vk::PhysicalDevice> devices = vkInst.enumeratePhysicalDevices();

    for(const auto& device : devices)
    {
        if(deviceIsSupported(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if(!physicalDevice)
        throw std::runtime_error("GPU not supported!");
}

bool Graphics::deviceIsSupported(const vk::PhysicalDevice& device) noexcept
{
    setQueueFamilies(device);

    bool extensionsSupported = deviceExtensionsSupported(device);
    bool swapChainCompatible;
    if(extensionsSupported)
    {
        setSwapChainDetails(device);
        swapChainCompatible = !swapChainDetails.formats.empty() && !swapChainDetails.modes.empty();
    }

    return queueIndices.found() && extensionsSupported && swapChainCompatible;
}

void Graphics::setQueueFamilies(const vk::PhysicalDevice& device) noexcept
{
    std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

    // searching of queues indices that support selected operations

    vk::Bool32 presentSupport = VK_FALSE;
    uint32_t i = 0;
    for(const auto& queueFamily : queueFamilies)
    {
        if(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            queueIndices.graphicFamily = i;
        
        if(device.getSurfaceSupportKHR(i, surface))
            queueIndices.presentFamily = i;

        if(queueIndices.found())
            break;

        i++;
    }
}

bool Graphics::deviceExtensionsSupported(const vk::PhysicalDevice& device) const noexcept
{
    std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for(const auto& extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

void Graphics::setSwapChainDetails(const vk::PhysicalDevice& device) noexcept
{
    swapChainDetails.capabilities = device.getSurfaceCapabilitiesKHR(surface);
    swapChainDetails.formats = device.getSurfaceFormatsKHR(surface);
    swapChainDetails.modes = device.getSurfacePresentModesKHR(surface);
}

void Graphics::initLogicalDevice()
{
    std::set<uint32_t> uniqueQueues = {queueIndices.graphicFamily.value(), queueIndices.presentFamily.value()};
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;
    for(auto queue : uniqueQueues)
    {
        vk::DeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = vk::StructureType::eDeviceQueueCreateInfo;
        queueCreateInfo.queueFamilyIndex = queue;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.emplace_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures = {};

    vk::DeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = vk::StructureType::eDeviceCreateInfo;
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
    
    try
    {
        device = physicalDevice.createDevice(deviceCreateInfo);
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << e.what() << '\n';
        throw std::runtime_error("Failed to create logical device!");
    }
    
// 0 because we are creating only 1 queue
    graphicQueue = device.getQueue(0, queueIndices.graphicFamily.value());
    presentQueue = device.getQueue(0, queueIndices.presentFamily.value());
}

void Graphics::initSwapChain()
{
    vk::Extent2D extent = getSwapExtent();
    vk::SurfaceFormatKHR surfaceFormat = getSwapSurfaceFormat();
    vk::PresentModeKHR presentMode = getSwapPresentMode();

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    uint32_t imageCount = swapChainDetails.capabilities.minImageCount + 1;

    if(
        swapChainDetails.capabilities.maxImageCount > 0 &&
        imageCount > swapChainDetails.capabilities.maxImageCount
    )
        imageCount = swapChainDetails.capabilities.maxImageCount;
    
    vk::SwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = vk::StructureType::eSwapchainCreateInfoKHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    createInfo.preTransform = swapChainDetails.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    uint32_t queueFamilyIndices[] = 
    {
        queueIndices.graphicFamily.value(), 
        queueIndices.presentFamily.value()
    };

    if(queueIndices.graphicFamily != queueIndices.presentFamily)
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 2;
    }

    try
    {
        swapChain = device.createSwapchainKHR(createInfo);
        swapChainImages = device.getSwapchainImagesKHR(swapChain); 
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << e.what() << '\n';
        throw std::runtime_error("Failed to create swap chain!");
    }
}

vk::Extent2D Graphics::getSwapExtent() const noexcept
{
    if(swapChainDetails.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return swapChainDetails.capabilities.currentExtent;
    else
    {
        int width, height;
        glfwGetFramebufferSize(wnd, &width, &height);

        vk::Extent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp
        (
            actualExtent.width,
            swapChainDetails.capabilities.minImageExtent.width, 
            swapChainDetails.capabilities.maxImageExtent.width
        );
        actualExtent.height = std::clamp
        (
            actualExtent.height,
            swapChainDetails.capabilities.minImageExtent.height, 
            swapChainDetails.capabilities.maxImageExtent.height
        );

        return actualExtent;
    }
}

vk::SurfaceFormatKHR Graphics::getSwapSurfaceFormat() const noexcept
{
    for(const auto& availableFormat : swapChainDetails.formats)
        if(
            availableFormat.format == vk::Format::eB8G8R8A8Srgb && 
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear
        )
            return availableFormat;

    return swapChainDetails.formats[0];
}

vk::PresentModeKHR Graphics::getSwapPresentMode() const noexcept
{
    for(const auto& availablePresentMode : swapChainDetails.modes)
        if(availablePresentMode == vk::PresentModeKHR::eMailbox)
            return availablePresentMode;

    return vk::PresentModeKHR::eFifo;
}

void Graphics::initImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for(uint32_t i = 0; i < swapChainImages.size(); i++)
    {
        vk::ImageViewCreateInfo createInfo = {};
        createInfo.sType = vk::StructureType::eImageViewCreateInfo;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = vk::ComponentSwizzle::eIdentity;
        createInfo.components.g = vk::ComponentSwizzle::eIdentity;
        createInfo.components.b = vk::ComponentSwizzle::eIdentity;
        createInfo.components.a = vk::ComponentSwizzle::eIdentity;
        createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        try
        {
            swapChainImageViews[i] = device.createImageView(createInfo);
        }
        catch(const std::runtime_error& e)
        {
            std::cerr << e.what() << '\n';
            throw std::runtime_error("Failed to create image views!");
        }
    }
}