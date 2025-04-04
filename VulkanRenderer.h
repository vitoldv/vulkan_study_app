#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <array>
#include "Mesh.h"

#include "VulkanUtils.h"

#ifdef NDEBUG
#define ENABLE_VALIDATION_LAYERS false
#else
#define ENABLE_VALIDATION_LAYERS true
#endif

// preferrable surface settings (selected if supported)
#define SURFACE_COLOR_FORMAT		VK_FORMAT_R8G8B8A8_UNORM
#define SURFACE_COLOR_SPACE			VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
#define SURFACE_PRESENTATION_MODE	VK_PRESENT_MODE_MAILBOX_KHR

#define MAX_FRAME_DRAWS 2

using namespace std;

// Names of extensions required to run the application
const vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};


class VulkanRenderer
{
private:
	GLFWwindow* window;

	int currentFrame = 0;

	// Native Vulkan Components
	VkInstance vkInstance; 
	VkPhysicalDevice vkPhysicalDevice;
	VkDevice vkLogicalDevice;
	VkQueue vkGraphicsQueue;
	VkQueue vkPresentationQueue;
	VkSurfaceKHR vkSurface;
	VkSwapchainKHR vkSwapchain;
	vector<SwapChainImage> swapchainImages;

	// Graphics pipeline
	VkRenderPass vkRenderPass;
	VkPipeline vkGraphicsPipeline;
	VkPipelineLayout vkPipelineLayout;
	vector<VkFramebuffer> vkSwapchainFramebuffers;
	VkCommandPool vkGraphicsCommandPool;
	vector<VkCommandBuffer> vkCommandBuffers;

	// Extension Vulkan Components
#ifndef NDEBUG
	VkDebugUtilsMessengerEXT debugMessenger;
#endif

	// Descriptors
	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkDescriptorPool vkDescriptorPool;
	vector<VkDescriptorSet> vkDescriptorSets;
	vector<VkBuffer> uniformBuffers;
	vector<VkDeviceMemory> uniformBuffersMemory;

	// Utility
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	vector<VkSemaphore> vkSemImageAvailable;
	vector<VkSemaphore> vkSemRenderFinished;
	vector<VkFence> vkDrawFences;

	// Scene
	glm::mat4 projectionMat;
	glm::mat4 viewMat;

	Mesh testMesh;

public:
	VulkanRenderer();

	int init(GLFWwindow* window);
	void draw();
	void cleanup();

	~VulkanRenderer();

private:
	
	void createVulkanInstance();
	void retrievePhysicalDevice();
	void createLogicalDevice();
	void createSurface();
	void createSwapChain();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSyncTools();
	void createDescriptorSetLayout();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();

	void setupDebugMessenger();

	VkSurfaceFormatKHR defineSurfaceFormat(const vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR definePresentationMode(const vector<VkPresentModeKHR> presentationModes);
	VkExtent2D defineSwapChainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	VkShaderModule createShaderModule(const vector<char>& code);
	void recordCommands();
	void updateUniformBuffers(uint32_t imageIndex);

	bool isInstanceExtensionsSupported(vector<const char*>* extensions);
	bool isDeviceSupportsRequiredExtensions(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);

	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
	SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);
	bool checkValidationLayerSupport();

	void printPhysicalDeviceInfo(VkPhysicalDevice device, bool printPropertiesFull = false, bool printFeaturesFull = false);
};

