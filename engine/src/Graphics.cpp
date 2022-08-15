#include "Graphics.h"
#include "dot_Exception.h"
#include "Shader.h"

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

    initRenderPass();
    initPipeline();
    
    initFramebuffers();

    initCommandPool();
    initCommandBuffer();

    initSyncObjects();
}

Graphics::~Graphics()
{
    device.waitIdle();

    destroySyncObjects();

    device.destroyCommandPool(commandPool);

    for(const auto& framebuffer : swapChainFramebuffers)
        device.destroyFramebuffer(framebuffer);

    device.destroyPipeline(pipeline);
    device.destroyPipelineLayout(pipelineLayout);
    device.destroyRenderPass(renderPass);

    for(const auto& imageView : swapChainImageViews)
        device.destroyImageView(imageView);

    device.destroySwapchainKHR(swapChain);
    device.destroy();
    vkInst.destroySurfaceKHR(surface);

    if(validationLayersEnabled)
        vkInst.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);

    vkInst.destroy();
}

void Graphics::beginFrame()
{
    if(device.waitForFences(1, &inFlightFence, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess);
    if(device.resetFences(1, &inFlightFence) != vk::Result::eSuccess);

    uint32_t imageIndex = device.acquireNextImageKHR(swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE).value;

    commandBuffer.reset();
    recordCommandBuffer(commandBuffer, imageIndex);

    vk::Semaphore waitSemaphores[] = {imageAvailableSemaphore};
    vk::Semaphore signalSemaphores[] = {renderFinishedSemaphore};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::SubmitInfo submitInfo = {};
    submitInfo.sType = vk::StructureType::eSubmitInfo;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if(graphicQueue.submit(1, &submitInfo, inFlightFence) != vk::Result::eSuccess);

    vk::SwapchainKHR swapChains[] = {swapChain};
    vk::PresentInfoKHR presentInfo = {};
    presentInfo.sType = vk::StructureType::ePresentInfoKHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    if(presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess);
}

void Graphics::endFrame()
{

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
        throw DOT_RUNTIME("Requested validation layers are not supported!")

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
        throw DOT_RUNTIME_WHAT(e);
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
    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
    vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;

    debugMessengerInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

    debugMessengerInfo.pfnUserCallback = debugCallback;
    debugMessengerInfo.pUserData = nullptr;
}

void Graphics::initSurface()
{
    VkSurfaceKHR vkSurface;

    if(glfwCreateWindowSurface(vkInst, wnd, nullptr, &vkSurface) != VK_SUCCESS)
        throw DOT_RUNTIME("Failed to create window surface!");

    surface = vk::SurfaceKHR(vkSurface);
}

void Graphics::selectPhysicalDevice()
{
    try
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
    }
    catch(const std::runtime_error& e)
    {
        throw DOT_RUNTIME_WHAT(e);
    }

    if(!physicalDevice)
        throw DOT_RUNTIME("GPU not supported!");
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
        throw DOT_RUNTIME_WHAT(e);
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
        throw DOT_RUNTIME_WHAT(e);
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
            throw DOT_RUNTIME_WHAT(e);
        }
    }
}

void Graphics::initRenderPass()
{
    vk::AttachmentDescription colorAttachment
    (
        vk::AttachmentDescriptionFlagBits::eMayAlias,   // flags
        swapChainImageFormat,                           // format
        vk::SampleCountFlagBits::e1,                    // samples
        vk::AttachmentLoadOp::eClear,                   // loadOp
        vk::AttachmentStoreOp::eStore,                  // storeOp
        vk::AttachmentLoadOp::eDontCare,                // stencilLoadOp
        vk::AttachmentStoreOp::eDontCare,               // stencilStoreOp
        vk::ImageLayout::eUndefined,                    // initialLayout
        vk::ImageLayout::ePresentSrcKHR                 // finalLayout
    );

    vk::AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    vk::SubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; 
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.srcAccessMask = vk::AccessFlagBits::eNone;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

    vk::RenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = vk::StructureType::eRenderPassCreateInfo;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    try
    {
        renderPass = device.createRenderPass(renderPassInfo);
    }
    catch(const std::runtime_error& e)
    {
        throw DOT_RUNTIME_WHAT(e);
    }
}

void Graphics::initPipeline()
{
    try
    {
        Shader vertShader("engine/shaders/vert.spv", device);
        Shader fragShader("engine/shaders/frag.spv", device);

        vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertShaderStageInfo.module = vertShader.getModule();
        vertShaderStageInfo.pName = "main";

        vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
        fragShaderStageInfo.module = fragShader.getModule();
        fragShaderStageInfo.pName = "main";
    
        vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

        vk::PipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        vk::PipelineVertexInputStateCreateInfo vertState = {};
        vertState.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
        vertState.vertexBindingDescriptionCount = 0;
        vertState.pVertexBindingDescriptions = nullptr;
        vertState.vertexAttributeDescriptionCount = 0;
        vertState.pVertexAttributeDescriptions = nullptr;

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // vk::Rect2D scissor = {};
        // scissor.offset = vk::Offset2D(0, 0);
        // scissor.extent = swapChainExtent;

        vk::PipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        vk::PipelineRasterizationStateCreateInfo rasterInfo = {};
        rasterInfo.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
        rasterInfo.depthClampEnable = VK_FALSE;
        rasterInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterInfo.polygonMode = vk::PolygonMode::eFill;
        rasterInfo.lineWidth = 1.0f;
        rasterInfo.cullMode = vk::CullModeFlagBits::eBack;
        rasterInfo.frontFace = vk::FrontFace::eClockwise;
        rasterInfo.depthBiasEnable = VK_FALSE;
        rasterInfo.depthBiasConstantFactor = 0.0f;
        rasterInfo.depthBiasClamp = 0.0f;
        rasterInfo.depthBiasSlopeFactor = 0.0f;

        vk::PipelineMultisampleStateCreateInfo multisampleInfo = {}; 
        multisampleInfo.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
        multisampleInfo.sampleShadingEnable = VK_FALSE;
        multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
        multisampleInfo.minSampleShading = 1.0f;
        multisampleInfo.pSampleMask = nullptr;
        multisampleInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleInfo.alphaToOneEnable = VK_FALSE; 

        vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | 
        vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd; 
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

        vk::PipelineColorBlendStateCreateInfo colorBlend = {};
        colorBlend.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
        colorBlend.logicOpEnable = VK_FALSE;
        colorBlend.logicOp = vk::LogicOp::eCopy;
        colorBlend.attachmentCount = 1;
        colorBlend.pAttachments = &colorBlendAttachment;
        colorBlend.blendConstants[0] = 0.0f;
        colorBlend.blendConstants[1] = 0.0f;
        colorBlend.blendConstants[2] = 0.0f;
        colorBlend.blendConstants[3] = 0.0f;

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);

        vk::GraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertState;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterInfo;
        pipelineInfo.pMultisampleState = &multisampleInfo;
        pipelineInfo.pDepthStencilState = nullptr;
        pipelineInfo.pColorBlendState = &colorBlend;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = nullptr;
        pipelineInfo.basePipelineIndex = -1;
    
        pipeline = device.createGraphicsPipeline(nullptr, pipelineInfo).value;
    }
    catch(const std::runtime_error& e)
    {
        throw DOT_RUNTIME_WHAT(e); 
    }
}

void Graphics::initFramebuffers()
{
    const size_t length = swapChainImageViews.size();
    swapChainFramebuffers.resize(length);

    for(size_t i = 0; i < length; i++)
    {
        vk::ImageView attachments[] = {swapChainImageViews[i]};

        vk::FramebufferCreateInfo createInfo = {};
        createInfo.sType = vk::StructureType::eFramebufferCreateInfo;
        createInfo.renderPass = renderPass;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = attachments;
        createInfo.width = swapChainExtent.width;
        createInfo.height = swapChainExtent.height;
        createInfo.layers = 1;

        try
        {
            swapChainFramebuffers[i] = device.createFramebuffer(createInfo);
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }
    }
}

void Graphics::initCommandPool()
{
    vk::CommandPoolCreateInfo createInfo = {};
    createInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
    createInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    createInfo.queueFamilyIndex = queueIndices.graphicFamily.value();

    try
    {
        commandPool = device.createCommandPool(createInfo);
    }
    catch(const std::runtime_error& e)
    {
        throw DOT_RUNTIME_WHAT(e);
    }
}

void Graphics::initCommandBuffer()
{
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    try
    {
        commandBuffer = device.allocateCommandBuffers(allocInfo).front();
    }
    catch(const std::runtime_error& e)
    {
        throw DOT_RUNTIME_WHAT(e);
    }
}

void Graphics::recordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex)
{
    vk::CommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    try
    {
        commandBuffer.begin(commandBufferBeginInfo);
    }
    catch(const std::runtime_error& e)
    {
        throw DOT_RUNTIME_WHAT(e);
    }

    vk::ClearValue clearColor;

    vk::RenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = vk::StructureType::eRenderPassBeginInfo;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassBeginInfo.renderArea.offset = vk::Offset2D(0, 0);
    renderPassBeginInfo.renderArea.extent = swapChainExtent;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

    viewport = vk::Viewport
    (
        0.0f,                           // x
        0.0f,                           // y
        (float)swapChainExtent.width,   // width
        (float)swapChainExtent.height,  // height
        0.0f,                           // minDepth
        1.0f                            // maxDepth
    );

    vk::Rect2D scissor = {};
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = swapChainExtent;

    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, scissor);

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();

    try
    {
        commandBuffer.end();
    }
    catch(const std::runtime_error& e)
    {
        throw DOT_RUNTIME_WHAT(e);
    }
}

void Graphics::initSyncObjects()
{
    vk::SemaphoreCreateInfo semInfo;

    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    try
    {
        imageAvailableSemaphore = device.createSemaphore(semInfo);
        renderFinishedSemaphore = device.createSemaphore(semInfo);
        inFlightFence = device.createFence(fenceInfo);
    }
    catch(const std::runtime_error& e)
    {
        throw DOT_RUNTIME_WHAT(e);
    }
}

void Graphics::destroySyncObjects()
{
    device.destroySemaphore(imageAvailableSemaphore);
    device.destroySemaphore(renderFinishedSemaphore);
    device.destroyFence(inFlightFence);
}