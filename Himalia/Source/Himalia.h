#pragma once

#include <Ganymede/Source/Ganymede.h>

#include <glm/glm.hpp>

#include <vector>
#include <filesystem>

GANYMEDE_ENUM(HimaliaVertexProperty,
    (Position)
    (Normal)
    (Color)
    (ColorRGBA)
    (Color8)
    (ColorRGBA8)
    (UV)
)

class HimaliaMesh
{
public:
    std::vector<glm::vec3> position;
    std::vector<glm::vec3> normal;
    std::vector<glm::vec4> color;
    std::vector<glm::vec2> uv;
    std::vector<uint32> indices;

    uint32 numVertices = 0;

    void BuildMesh(void* v, size_t stride, uint32 numProperty, HimaliaVertexProperty* properties, uint32* offsets = nullptr);
    template <typename vertexT> void BuildVertices(std::vector<vertexT>& vertexBuffer, uint32 numProperty, HimaliaVertexProperty* properties, uint32* offsets = nullptr)
    {
        vertexBuffer.resize(numVertices);
        BuildMesh(vertexBuffer.data(), sizeof(vertexT), numProperty, properties, offsets);
    }

    template <typename indexT> void BuildIndices(std::vector<indexT>& indexBuffer)
    {
        indexBuffer.resize(indices.size());
        for (uint32 i = 0; i < indices.size(); i++)
        {
            indexBuffer[i] = indexT(indices[i]);
        }
    }

    void BuildNormals();
};

class HimaliaPlyModel
{
public:
    HimaliaMesh mesh;

    void LoadFile(std::filesystem::path file);
};