#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

class Mesh
{

public:
	int id;
	std::string name;
    int textureIndex;

	Mesh();
	Mesh(int id, const char* name, std::vector<glm::vec3> vertices, std::vector<uint32_t> indices,
        std::vector<glm::vec2> texCoords, std::vector<glm::vec3> normals);
	~Mesh();

	std::vector<glm::vec3> getVertices();
	std::vector<glm::vec2> getTexCoords();
    std::vector<glm::vec3> getNormals();
	std::vector<uint32_t> getIndices();

    // Copy assignment operator
    Mesh& operator=(const Mesh& other) {
        if (this != &other) {
            id = other.id;
            name = other.name;
            vertices = other.vertices;
            indices = other.indices;
            texCoords = other.texCoords;
            normals = other.normals;
            textureIndex = other.textureIndex;
        }
        return *this;
    }

    // Move assignment operator
    Mesh& operator=(Mesh&& other) noexcept {
        if (this != &other) {
            id = other.id;
            name = std::move(other.name);
            vertices = std::move(other.vertices);
            indices = std::move(other.indices);
            texCoords = std::move(other.texCoords);
            normals = std::move(other.normals);
            textureIndex = other.textureIndex;
        }
        return *this;
    }

private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<uint32_t> indices;
};

