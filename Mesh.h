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
	Mesh(int id, const char* name, std::vector<glm::vec3> vertices, std::vector<uint32_t> indices);
	~Mesh();

	std::vector<glm::vec3> getVertices();
	std::vector<uint32_t> getIndices();

    // Copy assignment operator
    Mesh& operator=(const Mesh& other) {
        if (this != &other) {
            id = other.id;
            name = other.name;
            vertices = other.vertices;
            indices = other.indices;
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
        }
        return *this;
    }

private:
	std::vector<glm::vec3> vertices;
	std::vector<uint32_t> indices;
};

