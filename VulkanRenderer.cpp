#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer()
{
}

int VulkanRenderer::init(GLFWwindow* window)
{
	this->window = window;
	try
	{
		createVulkanInstance();
		setupDebugMessenger();
		createSurface();
		retrievePhysicalDevice();
		printPhysicalDeviceInfo(this->vkPhysicalDevice);
	}
	catch (const runtime_error &e)
	{
		printf("ERROR: %s\n", e.what());
		return EXIT_FAILURE;
	}

	return 0;
}

void VulkanRenderer::cleanup()
{
	vkDestroySurfaceKHR(this->vkInstance, this->vkSurface, nullptr);
	if (ENABLE_VALIDATION_LAYERS)
	{
		destroyDebugUtilsMessengerEXT(this->vkInstance, this->debugMessenger, nullptr);
	}
	vkDestroyDevice(this->vkLogicalDevice, nullptr);
	vkDestroyInstance(this->vkInstance, nullptr);
}

VulkanRenderer::~VulkanRenderer()
{
}

/// <summary>
/// Creates Vulkan Instance
/// </summary>
void VulkanRenderer::createVulkanInstance()
{
	// Create Info about the application (required for VInstanceCreateInfo)
	// P.S. Most data here doesn't affect the program itself but is helpful for developers
	VkApplicationInfo vkAppInfo = {};
	vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkAppInfo.pApplicationName = "Vulkan Renderer";
	vkAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	vkAppInfo.pEngineName = "VEngine";
	vkAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	vkAppInfo.apiVersion = VK_API_VERSION_1_4;

	// Create Info for Vulkan Instance creation
	VkInstanceCreateInfo vkInstanceInfo = {};
	vkInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceInfo.pApplicationInfo = &vkAppInfo;

	// Get required Vulkan extensions
	vector<const char*> instanceExtensions = vector<const char*>();
	uint32_t extensionsCount = 0;
	const char** extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
	for (int i = 0; i < extensionsCount; i++)
	{
		instanceExtensions.push_back(extensions[i]);
	}
	if (ENABLE_VALIDATION_LAYERS)
	{
		instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	vkInstanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	vkInstanceInfo.ppEnabledExtensionNames = instanceExtensions.data();

	// Checking whether extensions required by GLFW are supported by Vulkan Instance
	if (!isInstanceExtensionsSupported(&instanceExtensions))
	{
		throw runtime_error("Vulkan Instance doesn't support some extensions required by GLFW.");
	}

	// Setting Validation layers
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
	if (ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport())
	{
		throw runtime_error("Requested validations layers are not supported.");
	}
	if (ENABLE_VALIDATION_LAYERS)
	{
		vkInstanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		vkInstanceInfo.ppEnabledLayerNames = validationLayers.data();
		
		// This is the way to enable debug messanger for Vulkan instance creation and destroy functions
		// (messanger itself is created and destroyed in between those)
		populateDebugMessengerCreateInfo(debugCreateInfo);
		vkInstanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		vkInstanceInfo.enabledLayerCount = 0;
		vkInstanceInfo.ppEnabledLayerNames = nullptr;
	}
	// Create the Vulkan Instance
	VkResult result = vkCreateInstance(&vkInstanceInfo, nullptr, &this->vkInstance);
	if (result != VK_SUCCESS)
	{
		throw runtime_error("Failed to create a Vulkan Instance.");
	}
}

void VulkanRenderer::retrievePhysicalDevice()
{
	// Enumerate Vulkan accessible physical devices (GPUs)
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, nullptr);

	// Get the list of supported physical devices
	vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, devices.data());
	
	// TEMP: Simply pick the first device (preferrably a choice must be provided)
	if (isDeviceSuitable(devices[0]))
	{
		this->vkPhysicalDevice = devices[0];
	}

}

void VulkanRenderer::createLogicalDevice()
{
	// Get queue family indices for selected physical device
	QueueFamilyIndices indices = getQueueFamilies(this->vkPhysicalDevice);

	// Creating queue family info for queue family creation (considering that some indices may point
	// out to the same queue family so we create infos for only distinct ones ensuring using set)
	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	set<int> queueFamilyIndices = { indices.graphicsFamily, indices.presentationFamily };
	for (int queueFamilyIndex : queueFamilyIndices)
	{
		// Create info about Queue family logical device needs 
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount = 1;
		float priority = 1.0f;								// priority required for Vulkan to properly handle multiple queue families
		queueCreateInfo.pQueuePriorities = &priority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Information to create a Logical Device ("Device" shortened)
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());	// the number of Logical Devices Extensions (not the same extensions as ones for Vulkan Instance!)
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	VkPhysicalDeviceFeatures deviceFeatures = {};
	
	// Physical Devices features that Logical Device is going to use
	// TEMP: Empty (default) for now
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	VkResult result = vkCreateDevice(this->vkPhysicalDevice, &deviceCreateInfo, nullptr, &this->vkLogicalDevice);
	if (result != VK_SUCCESS)
	{
		throw runtime_error("Failed to create a Logical Device.");
	}

	// Save queues as they are created at the same time as the logical device
	vkGetDeviceQueue(this->vkLogicalDevice, indices.graphicsFamily, 0, &this->vkGraphicsQueue);
	vkGetDeviceQueue(this->vkLogicalDevice, indices.presentationFamily, 0, &this->vkPresentationQueue);
}

void VulkanRenderer::createSurface()
{
	if (glfwCreateWindowSurface(this->vkInstance, this->window, nullptr, &this->vkSurface) != VK_SUCCESS)
	{
		throw runtime_error("Failed to create window surface.");
	}

}

void VulkanRenderer::setupDebugMessenger()
{
	if (!ENABLE_VALIDATION_LAYERS)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	populateDebugMessengerCreateInfo(createInfo);

	if (createDebugUtilsMessengerEXT(this->vkInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to set up debug messenger.");
	}
}

/// <summary>
/// Checks whethers passed extensions are supported by Vulkan Instance.
/// </summary>
/// <param name="extensionsToCheck"></param>
/// <returns></returns>
bool VulkanRenderer::isInstanceExtensionsSupported(vector<const char*>* extensionsToCheck)
{
	// Getting the count of supported extensions
	uint32_t extensionsCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

	// Getting supported extensions list using retrieved extensions count
	vector<VkExtensionProperties> extensions(extensionsCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.data());

	// Checking whether all extensions to check are present within supported extensions
	for (const auto& extensionToCheck : *extensionsToCheck)
	{
		bool hasExtension = false;
		for (const auto& extenstion : extensions)
		{
			if (strcmp(extensionToCheck, extenstion.extensionName))
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
		{
			return false;
		}

		return true;
	}

	return false;
}

/// <summary>
/// Checks whether device supports specified and required extensions
/// </summary>
/// <param name="device"></param>
/// <returns></returns>
bool VulkanRenderer::isDeviceSupportsRequiredExtensions(VkPhysicalDevice device)
{
	// Get device extensions count
	uint32_t extensionsCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

	// If there is no extensions, then failure
	if (extensionsCount == 0)
	{
		return false;
	}

	// Populate device extensions
	vector<VkExtensionProperties> extensionsProperties(extensionsCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, extensionsProperties.data());

	for (const auto& deviceExtension : deviceExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensionsProperties)
		{
			if (strcmp(deviceExtension, extension.extensionName) == 0)
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
		{
			return false;
		}
	}

	return true;
}

bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device)
{
	//VkPhysicalDeviceProperties deviceProperties;
	//vkGetPhysicalDeviceProperties(device, &deviceProperties);

	//VkPhysicalDeviceFeatures deviceFeatures;
	//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	return getQueueFamilies(device).isValid()
		&& isDeviceSupportsRequiredExtensions(device)
		&& getSwapChainDetails(device).isValid();
}

QueueFamilyIndices VulkanRenderer::getQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	// Get all Queue Family Properties info for given physical device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

	// Go through each queue family and check if it has at least 1 of the required types of queue
	for (int i = 0; i < queueFamilyCount; i++)
	{
		auto queueFamily = queueFamilyProperties[i];

		// First check if queue family has at least 1 queue an this family (possibly none).
		// Queue can be multiple types defined by bitfield. We need to bitwise flags AND with VK_QUEUE_***_BIT
		// to check if it has required type.
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		// Checking if current Queue Family supports presentation (which is not a distinct queue family, can be graphics one)
		VkBool32 presentationSupport;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->vkSurface, &presentationSupport);
		if (queueFamily.queueCount > 0 && presentationSupport)
		{
			indices.presentationFamily = i;
		}


		if (indices.isValid())
		{
			break;
		}
	}

	return indices;
}

SwapChainDetails VulkanRenderer::getSwapChainDetails(VkPhysicalDevice device)
{
	SwapChainDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->vkSurface, &details.surfaceCapabilities);
	
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->vkSurface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.surfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->vkSurface, &formatCount, details.surfaceFormats.data());
	}

	uint32_t presentationModesCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->vkSurface, &presentationModesCount, nullptr);
	if (presentationModesCount > 0)
	{
		details.presentationModes.resize(presentationModesCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->vkSurface, &presentationModesCount, details.presentationModes.data());
	}

	return details;
}

bool VulkanRenderer::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

void VulkanRenderer::printPhysicalDeviceInfo(VkPhysicalDevice device, bool printPropertiesFull, bool printFeaturesFull)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	printf("%Device ID %d\n", deviceProperties.deviceID);
	printf("%s (%d)\n", deviceProperties.deviceName, deviceProperties.deviceType);
	printf("API version: %d\n", deviceProperties.apiVersion);
	printf("Driver version %d\n", deviceProperties.driverVersion);
	printf("Vendor ID %d\n", deviceProperties.vendorID);

	if (printPropertiesFull)
	{
		// TODO Print full properties
	}

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	
	// TODO Print device features info
}
