#include "VkMesh.h"

VkMesh::VkMesh()
{
	this->indexCount = 0;
	this->vertexCount = 0;
	this->vertexBuffer = VK_NULL_HANDLE;
	this->vertexBufferMemory = VK_NULL_HANDLE;
	this->indexBuffer = VK_NULL_HANDLE;
	this->indexBufferMemory = VK_NULL_HANDLE;
	this->physicalDevice = VK_NULL_HANDLE;
	this->logicalDevice = VK_NULL_HANDLE;
}

VkMesh::VkMesh(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkQueue transferQueue,
	VkCommandPool transferCommandPool, std::vector<Vertex>* vertices, std::vector<uint32_t>* indices, int textureIndex)
{
	this->indexCount = indices->size();
	this->vertexCount = vertices->size();
	this->physicalDevice = physicalDevice;
	this->logicalDevice = logicalDevice;
	this->textureIndex = textureIndex;
	createVertexBuffer(transferQueue, transferCommandPool, vertices);
	createIndexBuffer(transferQueue, transferCommandPool, indices);
}

VkMesh::~VkMesh() {}

int VkMesh::getVertexCount()
{
	return this->vertexCount;
}

VkBuffer VkMesh::getVertexBuffer()
{
	return this->vertexBuffer;
}

int VkMesh::getIndexCount()
{
	return this->indexCount;
}

VkBuffer VkMesh::getIndexBuffer()
{
	return this->indexBuffer;
}

int VkMesh::getTextureIndex()
{
	return this->textureIndex;
}


void VkMesh::destroyDataBuffers()
{
	vkDestroyBuffer(this->logicalDevice, this->indexBuffer, nullptr);
	vkFreeMemory(this->logicalDevice, this->indexBufferMemory, nullptr);

	vkDestroyBuffer(this->logicalDevice, this->vertexBuffer, nullptr);
	vkFreeMemory(this->logicalDevice, this->vertexBufferMemory, nullptr);
}

glm::mat4 VkMesh::getTransformMat()
{
	return this->transformMat;
}

void VkMesh::setTransformMat(glm::mat4 transform)
{
	this->transformMat = transform;
}

void VkMesh::createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices)
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

	copyBuffer(logicalDevice, transferQueue, transferCommandPool, stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

void VkMesh::createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices)
{
	// Size of buffer needed for indices
	VkDeviceSize bufferSize = sizeof(uint32_t) * indices->size();

	// Temporary buffer to stage indices data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(physicalDevice, logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	// MAP MEMORY TO STAGE BUFFER
	void* data;
	vkMapMemory(this->logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);			// "map" the indices buffer memory to some point
	memcpy(data, indices->data(), (size_t)(bufferSize));									// copy memory from indices vector to the point
	vkUnmapMemory(this->logicalDevice, stagingBufferMemory);								// unmap the indices buffer memory

	// Create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also indices buffer
	// Buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host))
	createBuffer(physicalDevice, logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &this->indexBuffer, &this->indexBufferMemory);

	copyBuffer(logicalDevice, transferQueue, transferCommandPool, stagingBuffer, this->indexBuffer, bufferSize);

	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}
