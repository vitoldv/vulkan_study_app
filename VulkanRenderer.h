#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <array>
#include "VkMesh.h"
#include "Mesh.h"
#include "VulkanUtils.h"
#include <map>
#include "stb_image.h"

#ifdef NDEBUG
#define ENABLE_VALIDATION_LAYERS false
#else
#define ENABLE_VALIDATION_LAYERS true
#endif

// preferrable surface settings (selected if supported)
#define SURFACE_COLOR_FORMAT		VK_FORMAT_R8G8B8A8_UNORM
#define SURFACE_COLOR_SPACE			VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
#define SURFACE_PRESENTATION_MODE	VK_PRESENT_MODE_MAILBOX_KHR

#define BACKGROUND_COLOR 0x008B8BFF

#define MAX_FRAME_DRAWS 2
#define MAX_OBJECTS 100


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
	VkPipeline vkSecondPipeline;
	VkPipelineLayout vkSecondPipelineLayout;

	vector<VkFramebuffer> vkSwapchainFramebuffers;
	VkCommandPool vkGraphicsCommandPool;
	vector<VkCommandBuffer> vkCommandBuffers;

	std::vector<VkImage> colorBufferImage;
	std::vector<VkDeviceMemory> colorBufferImageMemory;
	std::vector<VkImageView> colorBufferImageView;
	VkFormat colorFormat;

	std::vector<VkImage> depthBufferImage;
	std::vector<VkDeviceMemory> depthBufferImageMemory;
	std::vector<VkImageView> depthBufferImageView;
	VkFormat depthFormat;

	// Extension Vulkan Components
	VkDebugUtilsMessengerEXT debugMessenger;

	// Descriptors
	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkDescriptorSetLayout vkSamplerDescriptorSetLayout;
	VkDescriptorSetLayout vkInputDescriptorSetLayout;		// input to subpass 2
	VkDescriptorPool vkDescriptorPool;
	VkDescriptorPool vkInputDescriptorPool;
	vector<VkDescriptorSet> vkDescriptorSets;
	vector<VkDescriptorSet> vkSamplerDescriptorSets;
	vector<VkDescriptorSet> vkInputDescriptorSets;
	vector<VkBuffer> uniformBuffers;
	vector<VkDeviceMemory> uniformBuffersMemory;
	VkDeviceSize minUniformBufferOffset;
	VkPushConstantRange vkPushConstantRange;

	// LEFT FOR REFERENCE ON DYNAMIC UNIFORM BUFFERS
	// UboModel* modelTransferSpace;	
	// size_t modelUniformAlignment;
	//vector<VkBuffer> uniformBuffersDynamic;
	//vector<VkDeviceMemory> uniformBuffersMemoryDynamic;
	

	// Utility
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	vector<VkSemaphore> vkSemImageAvailable;
	vector<VkSemaphore> vkSemRenderFinished;
	vector<VkFence> vkDrawFences;

	// Scene
	glm::mat4 projectionMat;
	glm::mat4 viewMat;
	std::map<uint32_t, std::map<uint32_t, VkMesh>> modelsToRender;

	// Textures
	VkSampler vkTextureSampler;
	std::vector<VkImage> textureImages;
	std::vector<VkDeviceMemory> textureImageMemory;
	std::vector<VkImageView> textureImageViews;

public:
	VulkanRenderer();

	int init(GLFWwindow* window);
	void draw();
	//bool addToRenderer(Mesh* mesh, glm::vec3 color);
	bool addToRenderer(int modelId, int meshCount, Mesh* mesh, glm::vec3 color);
	bool addToRendererTextured(int modelId, int meshCount, Mesh* mesh, std::vector<std::string> textureFiles);
	bool updateModelTransform(int modelId, glm::mat4 newTransform);
	bool removeFromRenderer(int modelId);	
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
	void createColorBufferImage();
	void createDepthBuffer();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSyncTools();
	void createDescriptorSetLayout();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	void createInputDescriptorSets();
	void createPushConstantRange();
	void createTextureSampler();
	int createTextureSamplerDescriptor(VkImageView textureImageView);
	int createTextureImage(std::string fileName);
	int createTexture(std::string fileName);

	void setupDebugMessenger();

	VkSurfaceFormatKHR defineSurfaceFormat(const vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR definePresentationMode(const vector<VkPresentModeKHR> presentationModes);
	VkExtent2D defineSwapChainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);
	VkFormat defineSupportedFormat(const vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags userFlags,
		VkMemoryPropertyFlags propertyFlags, VkDeviceMemory* imageMemory);
	VkShaderModule createShaderModule(const vector<char>& code);
	void recordCommands(uint32_t currentImage);
	void updateUniformBuffers(uint32_t imageIndex);
	
	// LEFT FOR REFERENCE ON DYNAMIC UNIFORM BUFFERS
	//void allocateDynamicBufferTransferSpace();

	bool isInstanceExtensionsSupported(vector<const char*>* extensions);
	bool isDeviceSupportsRequiredExtensions(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);

	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
	SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);
	bool checkValidationLayerSupport();

	stbi_uc* loadTexture(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);

	void printPhysicalDeviceInfo(VkPhysicalDevice device, bool printPropertiesFull = false, bool printFeaturesFull = false);
};



// ------------------- TODO -------------------
// 1. Make a generic function for pipeline creation to avoid a lot of duplicated code in createGraphicsPipeline();
// 2. Create a solution for removing swapChainImages.size() overuse (Create a constant value for images count or 
// make a wrapper for single swapchain image code);
// 3. Think of optimal max descriptor sets count other than MAX_OBJECTS (probably also create a separate pool for
// sampler descriptors).
// 4. Think of single memory buffer for meshes related to specific model
// 5. Create solution for instance batching
// 6. Extract all duplicated "magic" Vulkan flags to single space in code