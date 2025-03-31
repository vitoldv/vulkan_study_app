#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <fstream>
#include <iostream>

using namespace std;

#define VALIDATION_LAYER_OUTPUT_STR "--- VALIDATION LAYER MSG: "
#define VALIDATION_LAYER_ALLOWED_MESSAGE_SEVERITY VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
#define VALIDATION_LAYER_ALLOWED_MESSAGE_TYPE VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT

// Indices (locations) of Queue Families (if they exist at all)
struct QueueFamilyIndices
{
	// location of Graphics queue family
	int graphicsFamily = -1;
	// location of Presentation queue family (likely to be the same as graphics family)
	int presentationFamily = -1;

	// checks if families are valid
	bool isValid()
	{
		return graphicsFamily >= 0 && presentationFamily >= 0;
	}
};

struct SwapChainDetails
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;		// surface properties like image size etc.
	vector<VkSurfaceFormatKHR> surfaceFormats;			// surface color formats like RGBA etc.
	vector<VkPresentModeKHR> presentationModes;			// specifies how images should be presented (translated) to the actual screen

	bool isValid()
	{
		return !surfaceFormats.empty() && !presentationModes.empty();
	}
};

struct SwapChainImage
{
	VkImage image;
	VkImageView imageView;
};

static vector<char> readFile(const string &filename)
{
	ifstream file(filename, ios::binary | ios::ate);
	if (!file.is_open())
	{
		throw runtime_error("Failed to open file.");
	}

	size_t fileSize = (size_t)file.tellg();
	vector<char> fileBuffer(fileSize);

	file.seekg(0);

	file.read(fileBuffer.data(), fileSize);

	file.close();

	return fileBuffer;
}


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{

	std::cerr << VALIDATION_LAYER_OUTPUT_STR << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VALIDATION_LAYER_ALLOWED_MESSAGE_SEVERITY;
	createInfo.messageType = VALIDATION_LAYER_ALLOWED_MESSAGE_TYPE;
	createInfo.pfnUserCallback = debugCallback;
}

static uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties)
{
	// Get preperties of physical device memory
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((allowedTypes & (1 << i))														// Index of memory type must match correspoding bit in allowedTypes
			&& (memProperties.memoryTypes[i].propertyFlags & properties) == properties)		// Desired property bit flags are part of memory type's property flags 	
		{
			// This memory type is valid, so return its index
			return i;
		}
	}
}

static void copyBuffer(VkDevice logicalDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
	VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize)
{
	// Command buffer to hold transfer commands
	VkCommandBuffer transferCommandBuffer;
	
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = transferCommandPool;
	allocInfo.commandBufferCount = 1;

	// Allocate command buffer from pool
	vkAllocateCommandBuffers(logicalDevice, &allocInfo, &transferCommandBuffer);

	// Info to begin command buffer record
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;		// We're only using the command buffer once

	// Record transfer commands
	vkBeginCommandBuffer(transferCommandBuffer, &beginInfo);
	
	// Region of data to copy from or to
	VkBufferCopy bufferCopyRegion = {};
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0; 
	bufferCopyRegion.size = bufferSize;

	// Command to copy src buffer to dst buffer
	vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);

	// End commands
	vkEndCommandBuffer(transferCommandBuffer);

	// submit to queue
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &transferCommandBuffer;

	// Submit transfer command to transfer queue and wait until it finishes
	vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(transferQueue);

	// Free temporary command buffer
	vkFreeCommandBuffers(logicalDevice, transferCommandPool, 1, &transferCommandBuffer);
}

static void createBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags,
	VkMemoryPropertyFlags bufferProperties, VkBuffer* buffer, VkDeviceMemory* bufferMemory)
{
	// CREATE VERTEX BUFFER
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.usage = bufferUsageFlags;											// Multiple types of buffer possible, we want Vertex Buffer
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;											// Similar to Swap Chain images, can share vertex buffers 

	VkResult result = vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, buffer);
	if (result != VK_SUCCESS)
	{
		throw runtime_error("Failed to create a vertex buffer.");
	}

	// GET BUFFER MEMORY REQUIREMENTS
	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memReq);

	// ALLOCATE MEMORY TO BUFFER
	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memReq.size;
	memAllocInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memReq.memoryTypeBits, bufferProperties);

	// Allocate memory to VKDeviceMemory
	result = vkAllocateMemory(logicalDevice, &memAllocInfo, nullptr, bufferMemory);
	if (result != VK_SUCCESS)
	{
		throw runtime_error("Failed to allocate Vertex Buffer Memory.");
	}

	// Allocate memory to given vertex buffer
	vkBindBufferMemory(logicalDevice, *buffer, *bufferMemory, 0);

}