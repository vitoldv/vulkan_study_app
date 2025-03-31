#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "VulkanUtils.h"

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
};

class Mesh
{

public:
	Mesh();
	Mesh(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, std::vector<Vertex>* vertices);
	~Mesh();

	int getVertexCount();
	VkBuffer getVertexBuffer();
	void destroyVertexBuffer();

private:
	int vertexCount;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;

	void createVertexBuffer(std::vector<Vertex>* vertices);
	uint32_t findMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags preperties);
};

