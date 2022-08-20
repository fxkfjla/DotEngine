#include "dot_Swapchain.h"
#include "dot_Exception.h"

#include <limits>
#include <algorithm>

#include <iostream>

namespace dot
{
    Swapchain::Swapchain(Window& wnd, Device& device)
        : wnd(wnd), device(device)
    {
        createSwapchain();
        createImageViews();
    }

    Swapchain::~Swapchain()
    {
        for(const auto& imageView : imageViews)
            device.getVkDevice().destroyImageView(imageView);

        device.getVkDevice().destroySwapchainKHR(swapchain);
    }

    void Swapchain::createSwapchain()
    {
        auto swapchainDetails = device.getSwapchainDetails();
        auto queueIndices = device.getQueueFamiliyIndices();
        
        extent = getExtent(swapchainDetails.capabilities);
        surfaceFormat = getSurfaceFormat(swapchainDetails.formats);
        imageFormat = surfaceFormat.format;
        presentMode = getPresentMode(swapchainDetails.modes);

        uint32_t imageCount = swapchainDetails.capabilities.minImageCount + 1;

        if(swapchainDetails.capabilities.maxImageCount > 0 && imageCount > swapchainDetails.capabilities.maxImageCount)
            imageCount = swapchainDetails.capabilities.maxImageCount;

        uint32_t queueFamilyIndices[] = {queueIndices.graphicFamily.value(), queueIndices.presentFamily.value()};
        
        vk::SwapchainCreateInfoKHR createInfo
        (
            vk::SwapchainCreateFlagsKHR(0U),                // flags 
            device.getSurface(),                            // surface
            imageCount,                                     // minImageCount
            imageFormat,                                    // imageFormat
            surfaceFormat.colorSpace,                       // imageColorSpace
            extent,                                         // imageExtent
            1,                                              // imageArrayLayers
            vk::ImageUsageFlagBits::eColorAttachment,       // imageUsage
            vk::SharingMode::eExclusive,                    // imageSharingMode
            nullptr,                                        // pQueueFamilyIndices
            swapchainDetails.capabilities.currentTransform, // preTransform
            vk::CompositeAlphaFlagBitsKHR::eOpaque,         // compositeAplha
            presentMode,                                    // presentMode
            VK_TRUE,                                        // clipped
            nullptr                                         // oldSwapchain        
        );

        if(queueIndices.graphicFamily != queueIndices.presentFamily)
        {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }

        try
        {
            swapchain = device.getVkDevice().createSwapchainKHR(createInfo);
            images = device.getVkDevice().getSwapchainImagesKHR(swapchain); 
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }
    }

    vk::Extent2D Swapchain::getExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const noexcept
    {
        if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;
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
                capabilities.minImageExtent.width, 
                capabilities.maxImageExtent.width
            );
            actualExtent.height = std::clamp
            (
                actualExtent.height,
                capabilities.minImageExtent.height, 
                capabilities.maxImageExtent.height
            );

            return actualExtent;
        }
    }

    vk::SurfaceFormatKHR Swapchain::getSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) const noexcept
    {
        for(const auto& availableFormat : formats)
            if(
                availableFormat.format == vk::Format::eB8G8R8A8Srgb && 
                availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear
            )
                return availableFormat;

        return formats[0];
    }

    vk::PresentModeKHR Swapchain::getPresentMode(const std::vector<vk::PresentModeKHR>& modes) const noexcept
    {
        for(const auto& availablePresentMode : modes)
            if(availablePresentMode == vk::PresentModeKHR::eMailbox)
                return availablePresentMode;

        return vk::PresentModeKHR::eFifo;
    }

    void Swapchain::createImageViews()
    {
        imageViews.resize(images.size());

        for(uint32_t i = 0; i < imageViews.size(); i++)
        {
            vk::ComponentMapping components;

            vk::ImageSubresourceRange subresourceRange
            (
                vk::ImageAspectFlagBits::eColor,    // aspectMask
                0,                                  // baseMipLevel  
                1,                                  // levelCount
                0,                                  // baseArrayLayer
                1                                   // layerCount
            );

            vk::ImageViewCreateInfo createInfo
            (
                vk::ImageViewCreateFlags(0U),   // flags
                images[i],                      // image
                vk::ImageViewType::e2D,         // viewType
                imageFormat,                    // imageFormat
                components,                     // components
                subresourceRange                // subresourceRange
            );

            try
            {
                imageViews[i] = device.getVkDevice().createImageView(createInfo);
            }
            catch(const std::runtime_error& e)
            {
                throw DOT_RUNTIME_WHAT(e);
            }
        }
    }
}