#include "dot_Swapchain.h"
#include "dot_Exception.h"

#include <limits>
#include <algorithm>

namespace dot
{
    Swapchain::Swapchain(Window& wnd, Device& device, std::shared_ptr<Swapchain> oldSwapchain)
        : wnd(wnd), device(device), oldSwapchain(oldSwapchain)
    {
        createSwapchain();
        createImageViews();
        createRenderPass();
        createFramebuffers();
        createSyncObjects();
    }

    Swapchain::~Swapchain()
    {
        destroySyncObjects();

        for(const auto& framebuffer : framebuffers)
            device.getVkDevice().destroyFramebuffer(framebuffer);

        device.getVkDevice().destroyRenderPass(renderPass);

        for(const auto& imageView : imageViews)
            device.getVkDevice().destroyImageView(imageView);

        device.getVkDevice().destroySwapchainKHR(swapchain);
    }

    vk::Result Swapchain::acquireNextImage(uint32_t& index) const
    {
        const vk::Device& device = this->device.getVkDevice();
        const size_t frame = currentFrameInFlight;
        const uint64_t max = std::numeric_limits<uint64_t>::max();

        device.waitForFences(imageInFlightFences[frame], VK_TRUE, max);

        // using vulkan c api to prevent from throwing an exception

        return vk::Result(vkAcquireNextImageKHR(device, swapchain, max, imageAvailableSemaphores[frame], nullptr, &index));
    }

    vk::Result Swapchain::submitCmdBuffer(const vk::CommandBuffer& cmdBuffer, uint32_t imageIndex)
    {
        device.getVkDevice().waitForFences(imageInFlightFences[currentFrameInFlight], VK_TRUE, std::numeric_limits<uint64_t>::max());

        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        vk::SubmitInfo submitInfo
        (
            imageAvailableSemaphores[currentFrameInFlight], // waitSemaphores
            waitStages,                                     // waitDstStageMask
            cmdBuffer,                                      // commandBuffers
            renderFinishedSemaphores[currentFrameInFlight]  // signalSemaphores
        );

        device.getVkDevice().resetFences(imageInFlightFences[currentFrameInFlight]);

        device.getGfxQueue().submit(submitInfo, imageInFlightFences[currentFrameInFlight]);

        // using vulkan c api to prevent from throwing an exception

        VkSemaphore waitSemaphores[] = {renderFinishedSemaphores[currentFrameInFlight]};
        VkSwapchainKHR swapchains[] = {swapchain};

        VkPresentInfoKHR vkPresentInfo
        {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapchains,
            .pImageIndices = &imageIndex
        };

        currentFrameInFlight = (currentFrameInFlight + 1) % maxFramesInFlight;

        return vk::Result(vkQueuePresentKHR(device.getPresentQueue(), &vkPresentInfo));
    }

    Swapchain::operator const vk::SwapchainKHR&() const noexcept
    {
        return swapchain;
    }

    const vk::Extent2D& Swapchain::getExtent() const noexcept
    {
        return extent;
    }

    const vk::RenderPass& Swapchain::getRenderPass() const noexcept
    {
        return renderPass;
    }

    const vk::Framebuffer& Swapchain::getFramebuffer(size_t index) const noexcept
    {
        return framebuffers[index];
    }

    size_t Swapchain::getMaxFramesInFlight() const noexcept
    {
        return maxFramesInFlight;
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

        maxFramesInFlight = imageCount;

        uint32_t queueFamilyIndices[] = {queueIndices.graphicFamily.value(), queueIndices.presentFamily.value()};
        
        vk::SwapchainCreateInfoKHR createInfo
        (
            vk::SwapchainCreateFlagsKHR(0U),                            // flags 
            device.getSurface(),                                        // surface
            imageCount,                                                 // minImageCount
            imageFormat,                                                // imageFormat
            surfaceFormat.colorSpace,                                   // imageColorSpace
            extent,                                                     // imageExtent
            1,                                                          // imageArrayLayers
            vk::ImageUsageFlagBits::eColorAttachment,                   // imageUsage
            vk::SharingMode::eExclusive,                                // imageSharingMode
            nullptr,                                                    // pQueueFamilyIndices
            swapchainDetails.capabilities.currentTransform,             // preTransform
            vk::CompositeAlphaFlagBitsKHR::eOpaque,                     // compositeAplha
            presentMode,                                                // presentMode
            VK_TRUE,                                                    // clipped
            oldSwapchain == nullptr ? nullptr : oldSwapchain->swapchain // oldSwapchain        
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
        int width, height;
        glfwGetFramebufferSize(wnd, &width, &height);

        vk::Extent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        return actualExtent;
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

    void Swapchain::createRenderPass()
    {
        vk::AttachmentDescription colorAttachment
        (
            vk::AttachmentDescriptionFlagBits::eMayAlias,   // flags
            imageFormat,                                    // format
            vk::SampleCountFlagBits::e1,                    // samples
            vk::AttachmentLoadOp::eClear,                   // loadOp
            vk::AttachmentStoreOp::eStore,                  // storeOp
            vk::AttachmentLoadOp::eDontCare,                // stencilLoadOp
            vk::AttachmentStoreOp::eDontCare,               // stencilStoreOp
            vk::ImageLayout::eUndefined,                    // initialLayout
            vk::ImageLayout::ePresentSrcKHR                 // finalLayout
        );

        vk::AttachmentReference colorAttachmentRef
        (
            0,                                          // attachment
            vk::ImageLayout::eColorAttachmentOptimal    // layout
        );

        auto colorAttachments = {colorAttachment};

        vk::SubpassDescription subpass
        (
            vk::SubpassDescriptionFlags(0U),    // flags
            vk::PipelineBindPoint::eGraphics,   // pipelineBindPoint
            {},                                 // inputAttachments
            colorAttachmentRef                  // colorAttachments
        );

        vk::SubpassDependency dependency
        (
            0U,                                                 // srcSubpass 
            0U,                                                 // dstSubpass
            vk::PipelineStageFlagBits::eColorAttachmentOutput,  // srcStageMask
            vk::PipelineStageFlagBits::eColorAttachmentOutput,  // dstStageMask
            vk::AccessFlagBits::eNone,                          // srcAccessMask      
            vk::AccessFlagBits::eColorAttachmentWrite,          // dstAccessMask
            vk::DependencyFlagBits::eByRegion                   // dependencyFlags
        );

        vk::RenderPassCreateInfo createInfo
        (
            vk::RenderPassCreateFlags(0U),  // flags
            colorAttachment,                // attachments
            subpass,                        // subpasses
            dependency                      // dependencies
        );

        try
        {
            renderPass = device.getVkDevice().createRenderPass(createInfo);
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }
    }

    void Swapchain::createFramebuffers()
    {
        const size_t length = imageViews.size();
        framebuffers.reserve(length);

        for(size_t i = 0; i < length; i++)
        {
            vk::FramebufferCreateInfo createInfo
            (
                vk::FramebufferCreateFlags(0U), // flags
                renderPass,                     // renderPass
                imageViews[i],                  // attachments
                extent.width,                   // width
                extent.height,                  // height
                1                               // layers 
            );

            try
            {
                framebuffers.emplace_back(device.getVkDevice().createFramebuffer(createInfo));
            }
            catch(const std::runtime_error& e)
            {
                throw DOT_RUNTIME_WHAT(e);
            }
        }
    }
    
    void Swapchain::createSyncObjects()
    {
        imageAvailableSemaphores.reserve(maxFramesInFlight);
        renderFinishedSemaphores.reserve(maxFramesInFlight);
        imageInFlightFences.reserve(maxFramesInFlight);

        vk::SemaphoreCreateInfo semaphoreInfo;
        vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);

        try
        {
            for(size_t i = 0; i < maxFramesInFlight; i++)
            {
                imageAvailableSemaphores.emplace_back(device.getVkDevice().createSemaphore(semaphoreInfo));
                renderFinishedSemaphores.emplace_back(device.getVkDevice().createSemaphore(semaphoreInfo));
                imageInFlightFences.emplace_back(device.getVkDevice().createFence(fenceInfo));
            }
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }
    }

    void Swapchain::destroySyncObjects()
    {
        for(size_t i = 0; i < maxFramesInFlight; i++)
        {
            device.getVkDevice().destroySemaphore(imageAvailableSemaphores[i]);
            device.getVkDevice().destroySemaphore(renderFinishedSemaphores[i]);
            device.getVkDevice().destroyFence(imageInFlightFences[i]);
        }
    }
}