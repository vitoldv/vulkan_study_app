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

struct UboModel
{
	glm::mat4 modelMat;
};

class VkMesh
{

public:
	VkMesh();
	VkMesh(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkQueue transferQueue,
		VkCommandPool transferCommandPool, std::vector<Vertex>* vertices, std::vector<uint32_t>* indices);
	~VkMesh();

	int getVertexCount();
	VkBuffer getVertexBuffer();
	int getIndexCount();
	VkBuffer getIndexBuffer();
	UboModel getModelMatrix();
	glm::mat4 getTransformMat();

	void setTransformMat(glm::mat4 transform);

	void destroyDataBuffers();

private:
	int vertexCount;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	int indexCount;
	VkBuffer indexBuffer; 
	VkDeviceMemory indexBufferMemory;

	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;

	// Transform
	glm::mat4 transformMat;

	UboModel uboModel;

	void createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices);
	void createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices);
};

