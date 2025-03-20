#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>
#include "VulkanUtils.h"

#ifdef NDEBUG
#define ENABLE_VALIDATION_LAYERS false
#else
#define ENABLE_VALIDATION_LAYERS true
#endif

using namespace std;

const vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};


class VulkanRenderer
{
private:
	GLFWwindow* window;

	// Native Vulkan Components
	VkInstance vkInstance; 
	VkPhysicalDevice vkPhysicalDevice;
	VkDevice vkLogicalDevice;
	VkQueue vkGraphicsQueue;

	// Extension Vulkan Components
#ifndef NDEBUG
	VkDebugUtilsMessengerEXT debugMessenger;
#endif

public:
	VulkanRenderer();

	int init(GLFWwindow* window);
	void cleanup();

	~VulkanRenderer();

private:
	
	void createVulkanInstance();
	void retrievePhysicalDevice();
	void createLogicalDevice();
	void setupDebugMessenger();

	bool isInstanceExtensionsSupported(vector<const char*>* extensions);
	bool isDeviceSupported(VkPhysicalDevice device);

	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
	bool checkValidationLayerSupport();

	void printPhysicalDeviceInfo(VkPhysicalDevice device, bool printPropertiesFull = false, bool printFeaturesFull = false);
};

