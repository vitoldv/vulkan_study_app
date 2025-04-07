#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include <iostream>
#include <vector>
#include <string>

#include "VulkanRenderer.h"

#define WINDOW_TITLE		"Vulkan Renderer"
#define WINDOW_WIDTH		800
#define WINDOW_HEIGHT		600

#define FPS_LIMIT			true
#define TARGET_FPS			60
#define TARGET_FRAME_TIME	(1000 / TARGET_FPS)

using namespace std;

GLFWwindow* window;
VulkanRenderer vulkanRenderer;

// time and fps
int previousFrameTime = 0;
int currentFrameTime = 0;
float deltaTime = 0;

Mesh mesh;
Mesh mesh2;
glm::vec3 meshPosition = {0.0, 0.0, 0.0};

void initWindow(string title, const int width, const int height)
{
	glfwInit();
	// Set GLFW to NOW work with OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

void processInput()
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		meshPosition.y += 3 * deltaTime;
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		meshPosition.y -= 3 * deltaTime;
	}
}

void update()
{
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), meshPosition * -1.0f);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), meshPosition);
	vulkanRenderer.updateMeshTransform(mesh.id, t1);
	vulkanRenderer.updateMeshTransform(mesh2.id, t2);
}

void render()
{
	vulkanRenderer.draw();
}

int main()
{
	mesh = Mesh(1, "testMesh", meshVertices, meshIndices);
	mesh2 = Mesh(2, "testMesh2", meshVertices, meshIndices);

	// Initialize window
	initWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	// Create and initialize Vulkan Renderer Instance
	if (vulkanRenderer.init(window) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	vulkanRenderer.addToRenderer(&mesh);
	vulkanRenderer.addToRenderer(&mesh2);

	float frameTime = 0;
	// Loop until window is closed
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		currentFrameTime = glfwGetTime() * 1000.0f;
		frameTime = currentFrameTime - previousFrameTime;

		if (FPS_LIMIT && frameTime < TARGET_FRAME_TIME) continue;

		deltaTime = frameTime / 1000.0f;
		previousFrameTime = currentFrameTime;

		processInput();
		update();
		render();
	}

	vulkanRenderer.cleanup();

	// Destroy window
	glfwDestroyWindow(window);
	glfwTerminate();
}