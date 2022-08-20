#pragma once

#include "dot_Vulkan.h"
#include "dot_Instance.h"

#include "Window.h"

#include <optional>

namespace dot
{
    class Device
    {
        struct SwapchainSupportDetails
        {
            vk::SurfaceCapabilitiesKHR capabilities;
            std::vector<vk::SurfaceFormatKHR> formats;
            std::vector<vk::PresentModeKHR> modes;
        } swapchainDetails;

        struct QueueFamilyIndices
        {
            std::optional<uint32_t> graphicFamily;
            std::optional<uint32_t> presentFamily;

            bool found() const noexcept
            {
                return
                    graphicFamily.has_value() &&
                    presentFamily.has_value();
            }
        } queueIndices;

    public:
        Device(Window&);
        Device(const Device&) = delete;
        Device(const Device&&) = delete;
        Device& operator=(const Device&) = delete;
        Device& operator=(const Device&&) = delete;
        ~Device();
        const SwapchainSupportDetails& getSwapchainDetails() const noexcept;
        const vk::SurfaceKHR& getSurface() const noexcept;
        const QueueFamilyIndices& getQueueFamiliyIndices() const noexcept;
        const vk::Device& getVkDevice() const noexcept;
    private:
        void createSurface();

        void selectPhysicalDevice();
        bool deviceSupported(const vk::PhysicalDevice&);
        void setQueueFamilies(const vk::PhysicalDevice&) noexcept;
        bool deviceExtensionsSupported(const vk::PhysicalDevice&) const;
        void setSwapchainDetails(const vk::PhysicalDevice&) noexcept;

        void createLogicalDevice();

        vk::SurfaceKHR surface;
        vk::PhysicalDevice physicalDevice;
        vk::Device device;
        vk::Queue graphicQueue;
        vk::Queue presentQueue;

        const std::vector<const char*> deviceExtensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        dot::Instance inst;
        Window& wnd;
    };
}