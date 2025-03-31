#pragma once

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