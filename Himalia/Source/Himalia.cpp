#include "Himalia.h"

#include <fstream>
#include <limits>

void HimaliaMesh::BuildMesh(void* v, size_t stride, uint32 numProperty, HimaliaVertexProperty* properties, uint32* offsets)
{
    for (uint32 i = 0; i < numVertices; i++)
    {
        size_t offset = 0;
        for (uint32 p = 0; p < numProperty; p++)
        {
            if (offsets) offset = offsets[p];
            switch (properties[p])
            {
            case HimaliaVertexProperty::Position:
                *(reinterpret_cast<glm::vec3*>((uint8*)(v) + stride * i + offset)) = position[i];
                if (!offsets) offset += sizeof(glm::vec3);
                break;
            case HimaliaVertexProperty::Normal:
                *(reinterpret_cast<glm::vec3*>((uint8*)(v) + stride * i + offset)) = normal[i];
                if (!offsets) offset += sizeof(glm::vec3);
                break;
            case HimaliaVertexProperty::Color:
                *(reinterpret_cast<glm::vec3*>((uint8*)(v) + stride * i + offset)) = glm::vec3(color[i]);
                if (!offsets) offset += sizeof(glm::vec3);
                break;
            case HimaliaVertexProperty::ColorRGBA:
                *(reinterpret_cast<glm::vec4*>((uint8*)(v) + stride * i + offset)) = color[i];
                if (!offsets) offset += sizeof(glm::vec4);
                break;
            case HimaliaVertexProperty::UV:
                *(reinterpret_cast<glm::vec2*>((uint8*)(v) + stride * i + offset)) = uv[i];
                if (!offsets) offset += sizeof(glm::vec2);
                break;
            }
        }
    }
}

void HimaliaMesh::BuildNormals()
{
    std::vector<float> weights;
    weights.resize(position.size(), 0.0);

    for (glm::vec3& n : normal) n = glm::vec3(0.0);

    for (uint32 i = 0; i < indices.size(); i += 3)
    {
        uint32 i0 = indices[i    ];
        uint32 i1 = indices[i + 1];
        uint32 i2 = indices[i + 2];

        glm::vec3 p0 = position[i0];
        glm::vec3 p1 = position[i1];
        glm::vec3 p2 = position[i2];

        glm::vec3 crossProduct = glm::cross(p1 - p0, p2 - p0);
        float w = glm::length(crossProduct);
        
        normal[i0] += crossProduct;
        normal[i1] += crossProduct;
        normal[i2] += crossProduct;

        weights[i0] += w;
        weights[i1] += w;
        weights[i2] += w;
    }

    for (uint32 i = 0; i < normal.size(); i += 3)
    {
        normal[i] = glm::normalize(normal[i]);
    }
}

GANYMEDE_ENUM(HimaliaPlyDataType,
    (Float)
    (Uchar)
    (Int)
)

void HimaliaPlyModel::LoadFile(std::filesystem::path filepath)
{
    std::fstream file;
    file.open(filepath, std::ios::in);

    struct Element
    {
        uint32 numElements = 0;
        bool isIndex = false;
        std::vector<HimaliaPlyDataType> types = {};
        std::vector<size_t> offsets = {};
    };

    struct RichVertex
    {
        glm::vec3 position = glm::vec3(std::numeric_limits<float>::quiet_NaN());
        glm::vec3 normal = glm::vec3(std::numeric_limits<float>::quiet_NaN());
        glm::vec4 color = glm::vec4(0.7, 0.7, 0.7, 1.0);
        glm::vec2 uv = glm::vec2(std::numeric_limits<float>::quiet_NaN());
    };

    std::vector<Element> elements;

    bool readHeaders = true;

    if (file.is_open())
    {
        std::string line;
        while (readHeaders && std::getline(file, line))
        {
            size_t pos = 0;
            std::vector<std::string> tokens;
            while ((pos = line.find(" ")) != std::string::npos) {
                std::string token = line.substr(0, pos);
                if (!token.empty()) tokens.push_back(token);
                line.erase(0, pos + 1);
            }
            if (!line.empty()) tokens.push_back(line);

            if (tokens.size() == 0) continue;

            // Parsing the line
            if (tokens[0].compare("element") == 0)
            {
                if (tokens.size() != 3)
                {
                    GanymedePrint "Element line has incorrect amount of tokens.";
                    return;
                }

                elements.emplace_back();
                uint32 index = uint32(elements.size() - 1);
                elements[index].isIndex = (tokens[1].compare("face") == 0);
                elements[index].numElements = std::stoi(tokens[2]);
            }
            else if (tokens[0].compare("property") == 0)
            {
                if (tokens.size() < 3)
                {
                    GanymedePrint "Property line has incorrect amount of tokens.";
                    return;
                }

                uint32 index = uint32(elements.size() - 1);
                if (elements[index].isIndex) continue;

                if (tokens[1].compare("float") == 0)
                    elements[index].types.push_back(HimaliaPlyDataType::Float);

                if (tokens[1].compare("uchar") == 0)
                    elements[index].types.push_back(HimaliaPlyDataType::Uchar);

                if (tokens[1].compare("int") == 0)
                    elements[index].types.push_back(HimaliaPlyDataType::Int);

                if (tokens[2].compare("x") == 0)
                    elements[index].offsets.push_back(offsetof(RichVertex, RichVertex::position) + offsetof(glm::vec3, glm::vec3::x));
                else if (tokens[2].compare("y") == 0)
                    elements[index].offsets.push_back(offsetof(RichVertex, RichVertex::position) + offsetof(glm::vec3, glm::vec3::y));
                else if (tokens[2].compare("z") == 0)
                    elements[index].offsets.push_back(offsetof(RichVertex, RichVertex::position) + offsetof(glm::vec3, glm::vec3::z));
                else if (tokens[2].compare("nx") == 0)
                    elements[index].offsets.push_back(offsetof(RichVertex, RichVertex::normal) + offsetof(glm::vec3, glm::vec3::x));
                else if (tokens[2].compare("ny") == 0)
                    elements[index].offsets.push_back(offsetof(RichVertex, RichVertex::normal) + offsetof(glm::vec3, glm::vec3::y));
                else if (tokens[2].compare("nz") == 0)
                    elements[index].offsets.push_back(offsetof(RichVertex, RichVertex::normal) + offsetof(glm::vec3, glm::vec3::z));
                else if (tokens[2].compare("s") == 0)
                    elements[index].offsets.push_back(offsetof(RichVertex, RichVertex::uv) + offsetof(glm::vec2, glm::vec2::s));
                else if (tokens[2].compare("t") == 0)
                    elements[index].offsets.push_back(offsetof(RichVertex, RichVertex::uv) + offsetof(glm::vec2, glm::vec2::t));
                else if (tokens[2].compare("red") == 0)
                    elements[index].offsets.push_back(offsetof(RichVertex, RichVertex::color) + offsetof(glm::vec4, glm::vec4::r));
                else if (tokens[2].compare("green") == 0)
                    elements[index].offsets.push_back(offsetof(RichVertex, RichVertex::color) + offsetof(glm::vec4, glm::vec4::g));
                else if (tokens[2].compare("blue") == 0)
                    elements[index].offsets.push_back(offsetof(RichVertex, RichVertex::color) + offsetof(glm::vec4, glm::vec4::b));
                else if (tokens[2].compare("alpha") == 0)
                    elements[index].offsets.push_back(offsetof(RichVertex, RichVertex::color) + offsetof(glm::vec4, glm::vec4::a));
                else
                    elements[index].offsets.push_back(0xFFFFFFFF);
            }
            else if (tokens[0].compare("end_header") == 0)
            {
                readHeaders = false;
                break;
            }
        }

        // Parsing actual elements
        for (Element& e : elements)
        {
            if (e.isIndex)
            {
                for (uint32 i = 0; i < e.numElements; i++)
                {
                    uint32 numFanIndices = 0;
                    file >> numFanIndices;

                    uint32 originIndex = 0;
                    uint32 firstIndex = 0;

                    for (uint32 j = 0; j < numFanIndices; j++)
                    {
                        uint32 index = 0;
                        file >> index;

                        if (j == 0) originIndex = index;

                        if (j >= 2)
                        {
                            mesh.indices.push_back(originIndex);
                            mesh.indices.push_back(firstIndex);
                            mesh.indices.push_back(index);
                        }

                        firstIndex = index;
                    }
                }
            }
            else
            {
                for (uint32 i = 0; i < e.numElements; i++)
                {
                    RichVertex v;

                    for (uint32 j = 0; j < e.offsets.size(); j++)
                    {
                        float value = 0.0;

                        if (e.types[j] == HimaliaPlyDataType::Uchar)
                        {
                            uint32 uintValue = 0;
                            file >> uintValue;
                            value = float(uintValue) / 255;
                        }
                        else if (e.types[j] == HimaliaPlyDataType::Int)
                        {
                            int32 intValue = 0;
                            file >> intValue;
                            value = float(intValue);
                        }
                        else if (e.types[j] == HimaliaPlyDataType::Float)
                        {
                            file >> value;
                        }
                    
                        if (e.offsets[j] != 0xFFFFFFFF) *(reinterpret_cast<float*>(reinterpret_cast<uint8*>(&v) + e.offsets[j])) = value;
                    }

                    mesh.position.push_back(v.position);
                    mesh.normal.push_back(v.normal);
                    mesh.color.push_back(v.color);
                    mesh.uv.push_back(v.uv);
                }
            }
        }

        mesh.numVertices = uint32(mesh.position.size());
    }

    file.close();
}
