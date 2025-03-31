#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, std::vector<Vertex>* vertices)
{
	this->vertexCount = vertices->size();
	this->physicalDevice = physicalDevice;
	this->logicalDevice = logicalDevice;
	createVertexBuffer(vertices);
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

void Mesh::createVertexBuffer(std::vector<Vertex>* vertices)
{
	// CREATE VERTEX BUFFER
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = sizeof(Vertex) * vertices->size();
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;				// Multiple types of buffer possible, we want Vertex Buffer
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;				// Similar to Swap Chain images, can share vertex buffers 
	
	VkResult result = vkCreateBuffer(this->logicalDevice, &bufferCreateInfo, nullptr, &this->vertexBuffer);
	if (result != VK_SUCCESS)
	{
		throw runtime_error("Failed to create a vertex buffer.");
	}

	// GET BUFFER MEMORY REQUIREMENTS
	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(this->logicalDevice, this->vertexBuffer, &memReq);

	// ALLOCATE MEMORY TO BUFFER
	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memReq.size;
	memAllocInfo.memoryTypeIndex = findMemoryTypeIndex(memReq.memoryTypeBits,			// index of memory type on physical device that has required bit flags
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT												// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT - CPU can interact with memory
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);										// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT - Allows placement of data straight into buffer after mapping (otherwise would hav eto specify manually)	

	// Allocate memory to VKDeviceMemory
	result = vkAllocateMemory(this->logicalDevice, &memAllocInfo, nullptr, &this->vertexBufferMemory);
	if (result != VK_SUCCESS)
	{
		throw runtime_error("Failed to allocate Vertex Buffer Memory.");
	}

	// Allocate memory to given vertex buffer
	vkBindBufferMemory(this->logicalDevice, this->vertexBuffer, this->vertexBufferMemory, 0);

	// MAP MEMORY TO VERTEX BUFFER
	void* data;
	vkMapMemory(this->logicalDevice, this->vertexBufferMemory, 0, bufferCreateInfo.size, 0, &data);		// "map" the vertex buffer memory to some point
	memcpy(data, vertices->data(), (size_t)(bufferCreateInfo.size));									// copy memory from vertices vector to the point
	vkUnmapMemory(this->logicalDevice, this->vertexBufferMemory);										// unmap the vertex buffer memory
}																		

uint32_t Mesh::findMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags properties)
{
	// Get preperties of physical device memory
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{ 
		if ((allowedTypes & (1 << i))													// Index of memory type must match correspoding bit in allowedTypes
			&& (memProperties.memoryTypes[i].propertyFlags & properties) == properties)	// Desired property bit flags are part of memory type's property flags 	
		{
			// This memory type is valid, so return its index
			return i;
		}
	}
}
