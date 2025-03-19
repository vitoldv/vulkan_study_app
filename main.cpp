#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include "VulkanRenderer.h"

#define WINDOW_TITLE	"Vulkan Renderer"
#define WINDOW_WIDTH	800
#define WINDOW_HEIGHT	600

using namespace std;

GLFWwindow* window;
VulkanRenderer vulkanRenderer;

void initWindow(string title, const int width, const int height)
{
	glfwInit();
	// Set GLFW to NOW work with OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

int main()
{
	// Initialize window
	initWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	// Create and initialize Vulkan Renderer Instance
	if (vulkanRenderer.init(window) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	// Loop until window is closed
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	vulkanRenderer.cleanup();

	// Destroy window
	glfwDestroyWindow(window);
	glfwTerminate();
}