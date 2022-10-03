#pragma once

#include "dot_Device.h"
#include "dot_Buffer.h"

#include <glm/glm.hpp>

#include <vector>
#include <memory>

namespace dot
{
    class Model
    {
    public:
        struct Vertex
        {
            glm::vec2 pos;
            glm::vec3 color;

            static std::vector<vk::VertexInputBindingDescription> getBindingDescription() noexcept;
            static std::vector<vk::VertexInputAttributeDescription> getAttributeDescription() noexcept;
        };
        Model(Device&, const std::vector<Vertex>&);
        void bind(const vk::CommandBuffer&) const noexcept;
    private:
        void createVertexBuffer(const std::vector<Vertex>&) noexcept;
        std::unique_ptr<Buffer> vertexBuffer;
        uint32_t vertexCount;

        Device& device;
    };
}