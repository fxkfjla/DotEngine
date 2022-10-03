#include "dot_Model.h"
#include "dot_Exception.h"

namespace dot
{
    std::vector<vk::VertexInputBindingDescription> Model::Vertex::getBindingDescription() noexcept
    {
        std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
        bindingDescriptions.reserve(1);

        bindingDescriptions.emplace_back(vk::VertexInputBindingDescription(0, sizeof(Model::Vertex)));

        return bindingDescriptions;
    }

    std::vector<vk::VertexInputAttributeDescription> Model::Vertex::getAttributeDescription() noexcept
    {
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
        attributeDescriptions.reserve(2);

        attributeDescriptions.emplace_back(vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos)));
        attributeDescriptions.emplace_back(vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)));

        return attributeDescriptions;
    }

    Model::Model(Device& device, const std::vector<Vertex>& verticies) 
        : device(device)
    {
        createVertexBuffer(verticies);
    }

    void Model::createVertexBuffer(const std::vector<Vertex>& verticies) noexcept
    {
        uint32_t vertexSize = sizeof(Vertex);
        vertexCount = static_cast<uint32_t>(verticies.size());
        vertexBuffer = std::make_unique<Buffer>
        (
            device, 
            vertexSize, vertexCount, 
            vk::BufferUsageFlagBits::eVertexBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );

        vertexBuffer->map();
        vertexBuffer->write((void*)verticies.data());
    } 

    void Model::bind(const vk::CommandBuffer& cmdBuffer) const noexcept
    {
        cmdBuffer.bindVertexBuffers(0, vertexBuffer->getVkBuffer(), {0});
    }
}