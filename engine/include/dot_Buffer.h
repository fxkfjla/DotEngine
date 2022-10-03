#pragma once

#include "dot_Device.h"

namespace dot
{
    class Buffer
    {
    public:
        Buffer
        (
            Device&, 
            const vk::DeviceSize& instanceSize, const vk::DeviceSize& instanceCount, 
            const vk::BufferUsageFlags& usage, const vk::MemoryPropertyFlags& memoryProperty
        );
        Buffer(Buffer&) = delete;
        Buffer(Buffer&&) = delete;
        Buffer& operator=(Buffer&) = delete;
        Buffer& operator=(Buffer&&) = delete;
        ~Buffer();
        operator const vk::Buffer&() const noexcept;
        void map(const vk::DeviceSize& size = VK_WHOLE_SIZE, const vk::DeviceSize& offset = 0) noexcept;
        void unmap() noexcept;
        void write(void* data, const vk::DeviceSize& size = VK_WHOLE_SIZE, const vk::DeviceSize& offset = 0) noexcept;
        const vk::Buffer& getVkBuffer() const noexcept;
    private:
        void createBuffer(const vk::BufferUsageFlags&, const vk::MemoryPropertyFlags&);
        void destroyBuffer() noexcept;
    public:
        vk::Buffer buffer;
        vk::DeviceMemory memory;
        vk::DeviceSize size;
        void* data = nullptr;

        Device& device;
    };
}