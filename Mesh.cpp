#include "Mesh.h"

Mesh::Mesh(int id, const char* name, std::vector<glm::vec3> vertices, std::vector<uint32_t> indices,
    std::vector<glm::vec2> texCoords)
{
    this->id = id;
    this->name = name;
    this->vertices = vertices;
    this->indices = indices;
    this->texCoords = texCoords;
}

Mesh::Mesh() 
{
}

Mesh::~Mesh()
{
}

std::vector<glm::vec3> Mesh::getVertices()
{
    return this->vertices;
}

std::vector<glm::vec2> Mesh::getTexCoords()
{
    return this->texCoords;
}

std::vector<uint32_t> Mesh::getIndices()
{
    return this->indices;
}
