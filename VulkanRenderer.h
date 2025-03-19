#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>
#include "VulkanUtils.h"

using namespace std;

class VulkanRenderer
{
private:
	GLFWwindow* window;

	// Native Vulkan Components
	VkInstance vkInstance; 
	VkPhysicalDevice vkPhysicalDevice;
	VkDevice vkLogicalDevice;
	VkQueue vkGraphicsQueue;

public:
	VulkanRenderer();

	int init(GLFWwindow* window);
	void cleanup();

	~VulkanRenderer();

private:
	
	void createVulkanInstance();
	void retrievePhysicalDevice();
	void createLogicalDevice();

	bool isInstanceExtensionsSupported(vector<const char*>* extensions);
	bool isDeviceSupported(VkPhysicalDevice device);

	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);

	void printPhysicalDeviceInfo(VkPhysicalDevice device, bool printPropertiesFull = false, bool printFeaturesFull = false);
};

