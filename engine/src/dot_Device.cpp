#include "dot_Device.h"
#include "dot_Exception.h"

#include <set>
#include <string>

namespace dot
{
    Device::Device(Window& wnd)
        : wnd(wnd)
    {
        createSurface();
        selectPhysicalDevice();
        createLogicalDevice();
        createCmdPoolGfx();
        createCmdPoolTransfer();
    }

    Device::~Device()
    {
        device.destroyCommandPool(cmdPoolTransfer);
        device.destroyCommandPool(cmdPoolGfx);
        device.destroy();
        inst.getVkInstance().destroySurfaceKHR(surface);
    }
    
    Device::operator const vk::Device&() const noexcept
    {
        return device;
    }

    vk::CommandBuffer Device::beginTransferCmd() const noexcept
    {
        vk::CommandBufferAllocateInfo allocInfo(cmdPoolTransfer, vk::CommandBufferLevel::ePrimary, 1);
        vk::CommandBuffer cmdBuffer = device.allocateCommandBuffers(allocInfo).front(); 

        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdBuffer.begin(beginInfo);

        return std::move(cmdBuffer);
    }

    void Device::endTransferCmd(const vk::CommandBuffer& cmdBuffer) const noexcept
    {
        cmdBuffer.end();

        vk::SubmitInfo submitInfo({}, {}, cmdBuffer);
        graphicQueue.submit(submitInfo);
        graphicQueue.waitIdle();

        device.freeCommandBuffers(cmdPoolTransfer, cmdBuffer);
    }

    void Device::copyBuffer(const vk::Buffer& src, const vk::Buffer& dst, const vk::DeviceSize& size) const noexcept
    {
        auto&& cmdBuffer = beginTransferCmd();

        vk::BufferCopy copy(0, 0, size);
        cmdBuffer.copyBuffer(src, dst, copy);

        endTransferCmd(cmdBuffer);
    }

    const Device::SwapchainSupportDetails& Device::getSwapchainDetails() const noexcept
    {
        return swapchainDetails;
    }

    const vk::SurfaceKHR& Device::getSurface() const noexcept
    {
        return surface;
    }

    const Device::QueueFamilyIndices& Device::getQueueFamiliyIndices() const noexcept
    {
        return queueIndices;
    }

    const vk::Device& Device::getVkDevice() const noexcept
    {
        return device;
    }

    const vk::Queue& Device::getGfxQueue() const noexcept
    {
        return graphicQueue;
    }

    const vk::Queue& Device::getPresentQueue() const noexcept
    {
        return presentQueue;
    }

    uint32_t Device::getMemoryType(uint32_t typeFilter, const vk::MemoryPropertyFlags& flags) const
    {
        vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

        for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            if(typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & flags) == flags)
                return i;

        throw std::runtime_error("Failed to find suitable memory type!");
    }

    const vk::CommandPool& Device::getCmdPoolGfx() const noexcept
    {
        return cmdPoolGfx;
    }

    void Device::createSurface()
    {
        VkSurfaceKHR vkSurface;

        if(glfwCreateWindowSurface(inst.getVkInstance(), wnd, nullptr, &vkSurface) != VK_SUCCESS)
            throw DOT_RUNTIME("Failed to create window surface!");

        surface = vk::SurfaceKHR(vkSurface);
    }

    void Device::selectPhysicalDevice()
    {
        try
        {
            std::vector<vk::PhysicalDevice> devices = inst.getVkInstance().enumeratePhysicalDevices();

            for(const auto& device : devices)
            {
                if(deviceSupported(device))
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

    bool Device::deviceSupported(const vk::PhysicalDevice& device)
    {
        setQueueFamilies(device);

        bool extensionsSupported = deviceExtensionsSupported(device);
        bool swapChainCompatible;
        if(extensionsSupported)
        {
            setSwapchainDetails(device);
            swapChainCompatible = !swapchainDetails.formats.empty() && !swapchainDetails.modes.empty();
        }

        return queueIndices.found() && extensionsSupported && swapChainCompatible;
    }

    void Device::setQueueFamilies(const vk::PhysicalDevice& device) noexcept
    {
        std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

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

    bool Device::deviceExtensionsSupported(const vk::PhysicalDevice& device) const
    {
        try
        {
            std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

            std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

            for(const auto& extension : availableExtensions)
                requiredExtensions.erase(extension.extensionName);

            return requiredExtensions.empty();
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }
    }

    void Device::setSwapchainDetails(const vk::PhysicalDevice& device) noexcept
    {

        swapchainDetails.capabilities = device.getSurfaceCapabilitiesKHR(surface);
        swapchainDetails.formats = device.getSurfaceFormatsKHR(surface);
        swapchainDetails.modes = device.getSurfacePresentModesKHR(surface);
    }

    void Device::createLogicalDevice()
    {
        std::set<uint32_t> uniqueQueues = {queueIndices.graphicFamily.value(), queueIndices.presentFamily.value()};

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        queueCreateInfos.reserve(uniqueQueues.size());

        float queuePriorities[] = {1.0f};
        for(auto queue : uniqueQueues)
        {
            vk::DeviceQueueCreateInfo queueCreateInfo
            (
                vk::DeviceQueueCreateFlags(0U),     // flags
                queue,                              // queueFamilyIndex
                queuePriorities                     // pQueuePriorities
            );

            queueCreateInfos.emplace_back(queueCreateInfo);
        }

        vk::PhysicalDeviceFeatures deviceFeatures = {};

        auto validationLayers = inst.getValidationLayers();

        vk::DeviceCreateInfo deviceCreateInfo
        (
            vk::DeviceCreateFlags(0U),  // flags
            queueCreateInfos            // pQueueCreateInfos
        );
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

        if(inst.validationLayersEnabled())
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
        
        graphicQueue = device.getQueue(0, queueIndices.graphicFamily.value());
        presentQueue = device.getQueue(0, queueIndices.presentFamily.value());
    }

    void Device::createCmdPoolGfx()
    {
        vk::CommandPoolCreateInfo createInfo
        (
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,     // flags
            queueIndices.graphicFamily.value()                      // queueFamilyIndex
        );

        try
        {
            cmdPoolGfx = device.createCommandPool(createInfo);
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }
    }

    void Device::createCmdPoolTransfer()
    {
        vk::CommandPoolCreateInfo createInfo(vk::CommandPoolCreateFlagBits::eTransient, queueIndices.presentFamily.value()); 

        try
        {
            cmdPoolTransfer = device.createCommandPool(createInfo);
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }
    }
}