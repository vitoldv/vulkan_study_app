#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkQueue transferQueue,
	VkCommandPool transferCommandPool, std::vector<Vertex>* vertices)
{
	this->vertexCount = vertices->size();
	this->physicalDevice = physicalDevice;
	this->logicalDevice = logicalDevice;
	createVertexBuffer(transferQueue, transferCommandPool, vertices);
}

Mesh::~Mesh()
{
}

int Mesh::getVertexCount()
{
	return vertexCount;
}

VkBuffer Mesh::getVertexBuffer()
{
	return vertexBuffer;
}

void Mesh::destroyVertexBuffer()
{
	vkDestroyBuffer(this->logicalDevice, this->vertexBuffer, nullptr);
	vkFreeMemory(this->logicalDevice, this->vertexBufferMemory, nullptr);
}

void Mesh::createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommanPool, std::vector<Vertex>* vertices)
{
	// Size of buffer needed for vertices
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();

	// Temporary buffer to stage vertex data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(physicalDevice, logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	// MAP MEMORY TO STAGE BUFFER
	void* data;
	vkMapMemory(this->logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);		// "map" the vertex buffer memory to some point
	memcpy(data, vertices->data(), (size_t)(bufferSize));									// copy memory from vertices vector to the point
	vkUnmapMemory(this->logicalDevice, stagingBufferMemory);										// unmap the vertex buffer memory

	// Create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also vertex buffer
	// Buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host))
	createBuffer(physicalDevice, logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory);

	copyBuffer(logicalDevice, transferQueue, transferCommanPool, stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}																		
