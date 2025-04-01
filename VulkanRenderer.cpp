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
		createLogicalDevice();
		createSwapChain();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();

		vector<Vertex> meshVertices = {
		{	{1, -1, 0.0},	{1, 0.0f, 0.0f}	},
		{	{1,  1, 0.0},	{0.0f, 1, 0.0f}	},
		{	{-1, 1, 0.0},	{0.0f, 0.0f, 1}	},
		};
		vector<uint32_t> meshIndices = {
			1, 2, 3
		};

		this->testMesh = Mesh(this->vkPhysicalDevice, this->vkLogicalDevice,
			vkGraphicsQueue, vkGraphicsCommandPool, &meshVertices, &meshIndices);

		createCommandBuffers();
		recordCommands();
		createSyncTools();


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
	// Wait until there is nothing on a queue 
	vkDeviceWaitIdle(this->vkLogicalDevice);

	testMesh.destroyDataBuffers();

	for (int i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		vkDestroySemaphore(this->vkLogicalDevice, this->vkSemRenderFinished[i], nullptr);
		vkDestroySemaphore(this->vkLogicalDevice, this->vkSemImageAvailable[i], nullptr);
		vkDestroyFence(this->vkLogicalDevice, this->vkDrawFences[i], nullptr);
	}

	vkDestroyCommandPool(this->vkLogicalDevice, vkGraphicsCommandPool, nullptr);
	for (auto framebuffer : vkSwapchainFramebuffers)
	{
		vkDestroyFramebuffer(this->vkLogicalDevice, framebuffer, nullptr);
	}
	vkDestroyPipeline(this->vkLogicalDevice, this->vkGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(this->vkLogicalDevice, this->vkPipelineLayout, nullptr);
	vkDestroyRenderPass(this->vkLogicalDevice, this->vkRenderPass, nullptr);
	for (auto image : swapchainImages)
	{
		vkDestroyImageView(this->vkLogicalDevice, image.imageView, nullptr);
	}
	vkDestroySwapchainKHR(this->vkLogicalDevice, this->vkSwapchain, nullptr);
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

void VulkanRenderer::createSwapChain()
{
	// Get Swap Chain details so we can pick best settings
	SwapChainDetails swapChainDetails = getSwapChainDetails(this->vkPhysicalDevice);

	// Find optimal surface values for our swap chain
	VkSurfaceFormatKHR surfaceFormat = defineSurfaceFormat(swapChainDetails.surfaceFormats);
	VkPresentModeKHR presentMode = definePresentationMode(swapChainDetails.presentationModes);
	VkExtent2D extent = defineSwapChainExtent(swapChainDetails.surfaceCapabilities);

	// How many images are in the swap chain? Get 1 more than the minimum to allow triple buffering
	uint32_t imageCount = swapChainDetails.surfaceCapabilities.minImageCount + 1;

	// If imageCount higher than max, then clamp down to max
	// If 0, then limitless
	if (swapChainDetails.surfaceCapabilities.maxImageCount > 0
		&& swapChainDetails.surfaceCapabilities.maxImageCount < imageCount)
	{
		imageCount = swapChainDetails.surfaceCapabilities.maxImageCount;
	}

	// Creation information for swap chain
	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = this->vkSurface;												// Swapchain surface
	swapChainCreateInfo.imageFormat = surfaceFormat.format;										// Swapchain format
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;								// Swapchain color space
	swapChainCreateInfo.presentMode = presentMode;												// Swapchain presentation mode
	swapChainCreateInfo.imageExtent = extent;													// Swapchain image extents
	swapChainCreateInfo.minImageCount = imageCount;												// Minimum images in swapchain
	swapChainCreateInfo.imageArrayLayers = 1;													// Number of layers for each image in chain
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;						// What attachment images will be used as
	swapChainCreateInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform;	// Transform to perform on swap chain images
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;						// How to handle blending images with external graphics (e.g. other windows)
	swapChainCreateInfo.clipped = VK_TRUE;														// Whether to clip parts of image not in view (e.g. behind another window, off screen, etc)

	// Get Queue Family Indices
	QueueFamilyIndices indices = getQueueFamilies(this->vkPhysicalDevice);

	// If Graphics and Presentation families are different, then swapchain must let images be shared between families
	if (indices.graphicsFamily != indices.presentationFamily)
	{
		// Queues to share between
		uint32_t queueFamilyIndices[] = {
			(uint32_t)indices.graphicsFamily,
			(uint32_t)indices.presentationFamily
		};

		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;		// Image share handling
		swapChainCreateInfo.queueFamilyIndexCount = 2;							// Number of queues to share images between
		swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;			// Array of queues to share between
	}
	else
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.queueFamilyIndexCount = 0;
		swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	// IF old swap chain been destroyed and this one replaces it, then link old one to quickly hand over responsibilities
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create Swapchain
	VkResult result = vkCreateSwapchainKHR(this->vkLogicalDevice, &swapChainCreateInfo, nullptr, &this->vkSwapchain);
	if (result != VK_SUCCESS)
	{
		throw runtime_error("Failed to create a Swapchain!");
	}

	// Store for later reference
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	// Get swap chain images (first count, then values)
	uint32_t swapChainImageCount;
	vkGetSwapchainImagesKHR(this->vkLogicalDevice, this->vkSwapchain, &swapChainImageCount, nullptr);
	vector<VkImage> images(swapChainImageCount);
	vkGetSwapchainImagesKHR(this->vkLogicalDevice, this->vkSwapchain, &swapChainImageCount, images.data());

	for (VkImage image : images)
	{
		// Store image handle
		SwapChainImage swapChainImage = {};
		swapChainImage.image = image;
		swapChainImage.imageView = createImageView(image, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		// Add to swapchain image list
		this->swapchainImages.push_back(swapChainImage);
	}
}

void VulkanRenderer::createRenderPass()
{
	// color attachment of render pass
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;						// Format to use for attachment
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;					// Number of samples to write for multisampling
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;				// Describes what to do with attachment before rendering
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;			// Describes what to do with attachment after rendering
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// Describes what to do with stencil before rendering
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	// Describes what to do with stencil after rendering

	// Framebuffer data will be stored as an image, but images can be given different data layouts
	// to give optimal use for certain operations
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// Image data layout before render pass starts
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		// Image data layout after render pass (to change to)

	// Attachment reference uses an attachment index that refers to index in the attachment list passed to renderPassCreateInfo
	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Information about a particular subpass the Render Pass is using
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;		// Pipeline type subpass is to be bound to
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;

	// Need to determine when layout transitions occur using subpass dependencies
	std::array<VkSubpassDependency, 2> subpassDependencies;

	// Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// Transition must happen after...
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;						// Subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;		// Pipeline stage
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;				// Stage access mask (memory access)
	// But must happen before...
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;


	// Conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// Transition must happen after...
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;;
	// But must happen before...
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = 0;

	// Create info for Render Pass
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachment;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	VkResult result = vkCreateRenderPass(this->vkLogicalDevice, &renderPassCreateInfo, nullptr, &this->vkRenderPass);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Render Pass.");
	}
}

void VulkanRenderer::createGraphicsPipeline()
{
	// read SPIR-V shader code
	auto vertexShaderCode = readFile("shaders/vert.spv");
	auto fragmentShaderCode = readFile("shaders/frag.spv");

	// build shader modules to link to graphics pipeline
	VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

	// VERTEX STAGE CREATION
	VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
	vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;			// shader stage name
	vertexShaderStageCreateInfo.module = vertexShaderModule;				// shader module to be used
	vertexShaderStageCreateInfo.pName = "main";								// shader enter function

	// FRAGMENT STAGE CREATION
	VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
	fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;		// shader stage name
	fragmentShaderStageCreateInfo.module = fragmentShaderModule;				// shader module to be used
	fragmentShaderStageCreateInfo.pName = "main";								// shader enter function

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

	// Describing vertex data layout
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	array<VkVertexInputAttributeDescription, 2> attributes;
	attributes[0].binding = 0;										// should be same as above
	attributes[0].location = 0;
	attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributes[0].offset = offsetof(Vertex, pos);

	attributes[1].binding = 0;										// should be same as above
	attributes[1].location = 1;
	attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributes[1].offset = offsetof(Vertex, color);

	// VERTEX INPUT
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;					// list of vertex binding descriptions (data spacing/stride information)
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributes.data();				// list of vertex attribute descriptions (data format and where to bind to/from)

	// INPUT ASSEMBLY
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// VIEWPORT & SCISSORS
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)this->swapChainExtent.width;
	viewport.height = (float)this->swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = this->swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	/*
	// DYNAMIC STATES
	// Dynamic states to enable
	vector<VkDynamicState> dynamicStatesEnables;
	dynamicStatesEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);	// allows to change viewport on runtime using vkCmdSetViewport
	dynamicStatesEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStatesEnables.size());
	dynamicStateCreateInfo.pDynamicStates = dynamicStatesEnables.data();
	*/

	// RASTERIZER
	VkPipelineRasterizationStateCreateInfo rastCreateInfo = {};
	rastCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rastCreateInfo.depthClampEnable = VK_FALSE;
	rastCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rastCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rastCreateInfo.lineWidth = 1.0f;
	rastCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rastCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rastCreateInfo.depthBiasEnable = VK_FALSE;				// whether to add depth bias to fragments (good for stopping "shadow acne" in shadow mapping)

	// MULTI SAMPLING
	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
	multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
	multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// BLENDING
	// Blending decides how to blend a new color being written to a fragment, with the old value

	// Blend Attachment State (how blending is handled)
	VkPipelineColorBlendAttachmentState colorState = {};
	colorState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT	// colors to apply blending to
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorState.blendEnable = VK_TRUE;													// Enable blending

	// Blending uses equation: (srcColorBlendFactor * new color) colorBlendOp (dstColorBlendFactor * old color)
	colorState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorState.colorBlendOp = VK_BLEND_OP_ADD;

	// Summarised: (VK_BLEND_FACTOR_SRC_ALPHA * new color) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * old color)
	//			   (new color alpha * new color) + ((1 - new color alpha) * old color)

	colorState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorState.alphaBlendOp = VK_BLEND_OP_ADD;
	// Summarised: (1 * new alpha) + (0 * old alpha) = new alpha

	VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};
	colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendingCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendingCreateInfo.attachmentCount = 1;
	colorBlendingCreateInfo.pAttachments = &colorState;

	// PIPELINE LAYOUT (TODO: Apply Future Descriptor Set Layouts) --
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	// Create Pipeline Layout
	VkResult result = vkCreatePipelineLayout(this->vkLogicalDevice, &pipelineLayoutCreateInfo, nullptr, &vkPipelineLayout);
	if (result != VK_SUCCESS)
	{
		throw runtime_error("Failed to create Pipeline Layout!");
	}

	// -- GRAPHICS PIPELINE CREATION --
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;									// Number of shader stages
	pipelineCreateInfo.pStages = shaderStages;							// List of shader stages
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;		// All the fixed function pipeline states
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.pRasterizationState = &rastCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
	pipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.layout = this->vkPipelineLayout;							// Pipeline Layout pipeline should use
	pipelineCreateInfo.renderPass = this->vkRenderPass;							// Render pass description the pipeline is compatible with
	pipelineCreateInfo.subpass = 0;							 			// Subpass of render pass to use with pipeline

	// Pipeline Derivatives : Can create multiple pipelines that derive from one another for optimisation
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;	// Existing pipeline to derive from...
	pipelineCreateInfo.basePipelineIndex = -1;				// or index of pipeline being created to derive from (in case creating multiple at once)

	// Create Graphics Pipeline
	result = vkCreateGraphicsPipelines(this->vkLogicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &this->vkGraphicsPipeline);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Graphics Pipeline!");
	}


	vkDestroyShaderModule(this->vkLogicalDevice, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(this->vkLogicalDevice, vertexShaderModule, nullptr);
}

void VulkanRenderer::createFramebuffers()
{
	vkSwapchainFramebuffers.resize(swapchainImages.size());
	for (int i = 0; i < vkSwapchainFramebuffers.size(); i++)
	{
		array<VkImageView, 1> attachmesnts = {
			swapchainImages[i].imageView
		};

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = this->vkRenderPass;
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachmesnts.size());
		framebufferCreateInfo.pAttachments = attachmesnts.data();								// list of attachements 1:1 with render pass
		framebufferCreateInfo.width = swapChainExtent.width;
		framebufferCreateInfo.height = swapChainExtent.height;
		framebufferCreateInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(this->vkLogicalDevice, &framebufferCreateInfo, nullptr, &vkSwapchainFramebuffers[i]);
		if (result != VK_SUCCESS)
		{
			throw runtime_error("Failed to create framebuffers.");
		}
	}
}

void VulkanRenderer::createCommandPool()
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = getQueueFamilies(this->vkPhysicalDevice).graphicsFamily;
	
	VkResult result = vkCreateCommandPool(this->vkLogicalDevice, &poolInfo, nullptr, &vkGraphicsCommandPool);
	if (result != VK_SUCCESS)
	{
		throw runtime_error("Failed to create Graphics Command Pool.");
	}
}

void VulkanRenderer::createCommandBuffers()
{
	this->vkCommandBuffers.resize(vkSwapchainFramebuffers.size());

	VkCommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool = vkGraphicsCommandPool;
	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;					// VK_COMMAND_BUFFER_LEVEL_PRIMARY		: buffer you submit directly to the queue. Can't be called by other buffers
																			// VK_COMMAND_BUFFER_LEVEL_SECONDARY	: buffer can't be called directly. Can be called from other buffers via vkCmdExecuteCommands when recording commands in primary buffer
	cbAllocInfo.commandBufferCount = static_cast<uint32_t>(this->vkCommandBuffers.size());

	VkResult result = vkAllocateCommandBuffers(this->vkLogicalDevice, &cbAllocInfo, this->vkCommandBuffers.data());
	if (result != VK_SUCCESS)
	{
		throw runtime_error("Failed to allocate command buffers");
	}
}

void VulkanRenderer::createSyncTools()
{
	this->vkSemImageAvailable.resize(MAX_FRAME_DRAWS);
	this->vkSemRenderFinished.resize(MAX_FRAME_DRAWS);
	this->vkDrawFences.resize(MAX_FRAME_DRAWS);

	VkSemaphoreCreateInfo semCreateInfo = {};
	semCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		if (vkCreateSemaphore(this->vkLogicalDevice, &semCreateInfo, nullptr, &vkSemImageAvailable[i]) != VK_SUCCESS
			|| vkCreateSemaphore(this->vkLogicalDevice, &semCreateInfo, nullptr, &vkSemRenderFinished[i]) != VK_SUCCESS
			|| vkCreateFence(this->vkLogicalDevice, &fenceCreateInfo, nullptr, &this->vkDrawFences[i]) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create semaphores and/or fence.");
		}
	}
}
	

VkSurfaceFormatKHR VulkanRenderer::defineSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	// If only 1 format available and is undefined, then this means ALL formats are available (no restrictions)
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { SURFACE_COLOR_FORMAT, SURFACE_COLOR_SPACE };
	}

	// If restricted, search for optimal format
	for (const auto& format : formats)
	{
		if (format.format == SURFACE_COLOR_FORMAT && format.colorSpace == SURFACE_COLOR_SPACE)
		{
			return format;
		}
	}

	// If can't find optimal format, then just return first format
	return formats[0];
}

void VulkanRenderer::recordCommands()
{
	VkCommandBufferBeginInfo bufferBeginInfo = {};
	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Information about how to begin a render pass (only required for graphical apps)
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = this->vkRenderPass;
	renderPassBeginInfo.renderArea.offset = { 0,0 };						// start point of render pass
	renderPassBeginInfo.renderArea.extent = swapChainExtent;
	VkClearValue clearValues[]{
		{0.6f, 0.65f, 0.4f, 1.0f}
	};
	renderPassBeginInfo.pClearValues = clearValues;							// list of clear values (TODO: depth attachment clear value)
	renderPassBeginInfo.clearValueCount = 1;

	for (int i = 0; i < this->vkCommandBuffers.size(); i++)
	{
		renderPassBeginInfo.framebuffer = this->vkSwapchainFramebuffers[i];

		// Start recording commands to command buffer 
		VkResult result = vkBeginCommandBuffer(this->vkCommandBuffers[i], &bufferBeginInfo);
		if (result != VK_SUCCESS)
		{
			throw runtime_error("Failed to start recording a command buffer.");
		}

		// Begin render pass
		vkCmdBeginRenderPass(this->vkCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);;

		// bind pipeline to be used with render pass
		vkCmdBindPipeline(this->vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->vkGraphicsPipeline);

		VkBuffer vertexBuffers[] = { testMesh.getVertexBuffer() };								// buffers to bind
		VkBuffer indexBuffer = testMesh.getIndexBuffer();
		VkDeviceSize offsets[] = { 0 };															// offsets into buffers being bound
		vkCmdBindVertexBuffers(this->vkCommandBuffers[i], 0, 1, vertexBuffers, offsets);		// Command to bind vertex buffer before deawing with them
		vkCmdBindIndexBuffer(this->vkCommandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// execute pipeline
		vkCmdDrawIndexed(this->vkCommandBuffers[i], static_cast<uint32_t>(testMesh.getIndexCount()), 1, 0, -1, 0);

		// End render pass
		vkCmdEndRenderPass(this->vkCommandBuffers[i]);

		// Stop recording commands to command buffer 
		result = vkEndCommandBuffer(this->vkCommandBuffers[i]);
		if (result != VK_SUCCESS)
		{
			throw runtime_error("Failed to stop recording a command buffer.");
		}
	}
}

VkPresentModeKHR VulkanRenderer::definePresentationMode(const std::vector<VkPresentModeKHR> presentationModes)
{
	// Look for Mailbox presentation mode
	for (const auto& presentationMode : presentationModes)
	{
		if (presentationMode == SURFACE_PRESENTATION_MODE)
		{
			return presentationMode;
		}
	}

	// If can't find, use FIFO as Vulkan spec says it must be present
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::defineSwapChainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	// If current extent is at numeric limits, then extent can vary. Otherwise, it is the size of the window.
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return surfaceCapabilities.currentExtent;
	}
	else
	{
		// If value can vary, need to set manually

		// Get window size
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		// Create new extent using window size
		VkExtent2D newExtent = {};
		newExtent.width = static_cast<uint32_t>(width);
		newExtent.height = static_cast<uint32_t>(height);

		// Surface also defines max and min, so make sure within boundaries by clamping value
		newExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
		newExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

		return newExtent;
	}
}

void VulkanRenderer::draw()
{
	// 1 Get next available image to draw to and set something to signal when we're finished with the image (a semaphore)
	// 2 Submit command buffer to queue for execution,  making sure it waits for the image to e signalled as available before drawing
	// and signals when it ahas finished rendering
	// 3 Present image to screen when it has signalled finished rendering

	// Wait for given fence to signal open from last draw before continuing
	vkWaitForFences(this->vkLogicalDevice, 1, &vkDrawFences[currentFrame], VK_TRUE, numeric_limits<uint64_t>::max());
	// Reset fence
	vkResetFences(this->vkLogicalDevice, 1, &vkDrawFences[currentFrame]);

	// -- 1
	uint32_t imageIndex;
	vkAcquireNextImageKHR(this->vkLogicalDevice, this->vkSwapchain, numeric_limits<uint64_t>::max(), this->vkSemImageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

	// -- 2
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;									// num of semaphores to wait for
	submitInfo.pWaitSemaphores = &vkSemImageAvailable[currentFrame];					// list of semaphores
	VkPipelineStageFlags waitStages[] = {				
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};
	submitInfo.pWaitDstStageMask = waitStages;							// stages to check semaphores at
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->vkCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;								// number of semaphores to signal
	submitInfo.pSignalSemaphores = &this->vkSemRenderFinished[currentFrame];

	VkResult result = vkQueueSubmit(this->vkGraphicsQueue, 1, &submitInfo, vkDrawFences[currentFrame]);
	if (result != VK_SUCCESS)
	{
		throw runtime_error("Failed to submit Comand buffer to Graphics Queue.");
	}

	// -- 3
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &vkSemRenderFinished[currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &this->vkSwapchain;
	presentInfo.pImageIndices = &imageIndex;					// index of images in swapchain to present

	result = vkQueuePresentKHR(this->vkPresentationQueue, &presentInfo);
	if (result != VK_SUCCESS)
	{
		throw runtime_error("Failed to present image.");
	}

	// Get next frame (use % MAX_FRAME_DRAWS to keep value below MAX_FRAME_DRAWS)
	currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;
}


VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;	// Allows remapping of rgba components to other rgba values
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	// subresources allow to view only selected part of an image
	imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;		// which aspect of image to view (COLOR_BIT for color)
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;				// Start mipmap level to view from
	imageViewCreateInfo.subresourceRange.levelCount = 1;				// number of mipmap levels to view
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;			// start array level to view from
	imageViewCreateInfo.subresourceRange.layerCount = 1;				// number of array layers to view

	VkImageView imageView;
	VkResult result = vkCreateImageView(this->vkLogicalDevice, &imageViewCreateInfo, nullptr, &imageView);
	if (result != VK_SUCCESS)
	{
		throw runtime_error("Failed to create an ImageView.");
	}

	return imageView;

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
	SwapChainDetails details = {};

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

VkShaderModule VulkanRenderer::createShaderModule(const vector<char>& code)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = code.size();
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(this->vkLogicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS)
	{
		throw runtime_error("Failed to create shader module.");
	}

	return shaderModule;
}
