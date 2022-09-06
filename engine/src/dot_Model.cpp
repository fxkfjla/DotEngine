#include "dot_Model.h"

namespace dot
{
    std::vector<vk::VertexInputBindingDescription> Model::Vertex::getBindingDescription() noexcept
    {
        return std::vector<vk::VertexInputBindingDescription>(0U, sizeof(dot::Model::Vertex));
    }

    std::vector<vk::VertexInputAttributeDescription> Model::Vertex::getAttributeDescription() noexcept
    {
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
        attributeDescriptions.reserve(2);

        attributeDescriptions.emplace_back(vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos)));
        attributeDescriptions.emplace_back(vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)));

        return attributeDescriptions;
    }
}