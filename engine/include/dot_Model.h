#pragma once

#include "dot_Vulkan.h"

#include <glm/glm.hpp>

#include <vector>

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
    private:
        std::vector<Vertex> verticies =
        {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
    };
}