#include "dot_Buffer.h"
#include "dot_Exception.h"

namespace dot
{
    Buffer::Buffer
    (
        Device& device, 
        const vk::DeviceSize& instanceSize, const vk::DeviceSize& instanceCount, 
        const vk::BufferUsageFlags& usage, const vk::MemoryPropertyFlags& memoryProperty
    ) 
        : device(device)
    {
        size = instanceSize * instanceCount;
        createBuffer(usage, memoryProperty);
    }

    Buffer::~Buffer()
    {
        unmap();
        destroyBuffer();
    }

    void Buffer::map(const vk::DeviceSize& size, const vk::DeviceSize& offset) noexcept
    {
        data = device.getVkDevice().mapMemory(memory, offset, size);
    }

    void Buffer::unmap() noexcept
    {
        if(data)
        {
            device.getVkDevice().unmapMemory(memory);
            data = nullptr;
        }
    }

    void Buffer::write(void* data, const vk::DeviceSize& size, const vk::DeviceSize& offset) noexcept
    {
        if(size == VK_WHOLE_SIZE)
            memcpy(this->data, data, this->size);
        else
        {
            char* memOffset = static_cast<char*>(this->data);
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

    const vk::Buffer& Buffer::getVkBuffer() const noexcept
    {
        return buffer;
    }

    void Buffer::createBuffer(const vk::BufferUsageFlags& usage, const vk::MemoryPropertyFlags& memoryProperty)
    {
        vk::BufferCreateInfo createInfo(vk::BufferCreateFlags(0U), size, usage, vk::SharingMode::eExclusive);
        try
        {
            buffer = device.getVkDevice().createBuffer(createInfo); 
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }

        vk::MemoryRequirements memRequirements = device.getVkDevice().getBufferMemoryRequirements(buffer);
        try
        {
            uint32_t memTypeIndex = device.getMemoryType(memRequirements.memoryTypeBits, memoryProperty);
            vk::MemoryAllocateInfo allocateInfo(memRequirements.size, memTypeIndex);

            memory = device.getVkDevice().allocateMemory(allocateInfo);
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }

        device.getVkDevice().bindBufferMemory(buffer, memory, 0);
    }

    void Buffer::destroyBuffer() noexcept
    {
        device.getVkDevice().destroyBuffer(buffer);
        device.getVkDevice().freeMemory(memory); 
    }
}