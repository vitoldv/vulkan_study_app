#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include "VulkanRenderer.h"

#define WINDOW_TITLE	"Vulkan Renderer"
#define WINDOW_WIDTH	800
#define WINDOW_HEIGHT	600

#define FPS_LIMIT 1
#define TARGET_FPS 60
#define TARGET_FRAME_TIME (1000 / TARGET_FPS)

using namespace std;

GLFWwindow* window;
VulkanRenderer vulkanRenderer;

// time and fps
int previousFrameTime = 0;
int currentFrameTime = 0;
float deltaTime = 0;

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

	float frameTime = 0;
	// Loop until window is closed
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		vulkanRenderer.draw();
		
		currentFrameTime = glfwGetTime() * 1000.0f;
		frameTime = currentFrameTime - previousFrameTime;

		if (FPS_LIMIT == 1 && frameTime < TARGET_FRAME_TIME) continue;

		deltaTime = frameTime / 1000.0f;
		previousFrameTime = currentFrameTime;
	}

	vulkanRenderer.cleanup();

	// Destroy window
	glfwDestroyWindow(window);
	glfwTerminate();
}