#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

class Mesh
{

public:
	int id;
	std::string name;

	Mesh();
	Mesh(int id, const char* name, std::vector<glm::vec3> vertices, std::vector<uint32_t> indices,
        std::vector<glm::vec2> texCoords);
	~Mesh();

	std::vector<glm::vec3> getVertices();
	std::vector<glm::vec2> getTexCoords();
	std::vector<uint32_t> getIndices();

    // Copy assignment operator
    Mesh& operator=(const Mesh& other) {
        if (this != &other) {
            id = other.id;
            name = other.name;
            vertices = other.vertices;
            indices = other.indices;
            texCoords = other.texCoords;
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
        }
        return *this;
    }

private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texCoords;
	std::vector<uint32_t> indices;
};

