#pragma once

#include "glm/glm.hpp"
#include "NonCopyable.h"
#include "NonMovable.h"
#include <vector>
#include <string>

struct Vertex
{
    Vertex();

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> textCoords;
};

struct Texture 
{
    Texture();

    unsigned int ID;
    std::string type;
};

class ShaderHandler;
struct Mesh final : private NonCopyable, private NonMovable
{
    Mesh();
    ~Mesh();

    void render(ShaderHandler& shaderHandler) const;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    unsigned int vaoID;
    unsigned int vboID;
    unsigned int indiciesID;
};