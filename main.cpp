#define GLFW_INCLUDE_VULKAN
#define STB_IMAGE_IMPLEMENTATION

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <vector>
#include <string>

#include "VulkanRenderer.h"

#define WINDOW_TITLE		"Vulkan Renderer"
#define WINDOW_WIDTH		1920
#define WINDOW_HEIGHT		1080

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

int modelId;
float angleRot = 0;

std::vector<Mesh> importModel(std::string fileName)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

	std::vector<Mesh> model(scene->mNumMeshes);
	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		auto meshData = scene->mMeshes[i];
		std::vector<glm::vec3> vertices(meshData->mNumVertices);
		std::vector<glm::vec3> normals(meshData->mNumVertices);
		std::vector<glm::vec2> texCoords(meshData->mNumVertices);
		std::vector<uint32_t> indices(meshData->mNumFaces * 3);
		for (int j = 0; j < meshData->mNumVertices; j++)
		{
			vertices[j] = glm::vec3(meshData->mVertices[j].x, meshData->mVertices[j].y, meshData->mVertices[j].z);
			normals[j] = glm::vec3(meshData->mNormals[j].x, meshData->mNormals[j].y, meshData->mNormals[j].z);
			texCoords[j] = glm::vec2(meshData->mTextureCoords[0][j].x, meshData->mTextureCoords[0][j].y);
		}
		for (int j = 0; j < meshData->mNumFaces; j++)
		{
			auto face = meshData->mFaces[j];
			indices[j * 3] = face.mIndices[0] + 1;
			indices[j * 3 + 1] = face.mIndices[1] + 1;
			indices[j * 3 + 2] = face.mIndices[2] + 1;
		}
		Mesh mesh = Mesh(i, meshData->mName.C_Str(), vertices, indices, texCoords, normals);
		model[i] = mesh;
	}

	return model;
}

void initWindow(string title, const int width, const int height)
{
	glfwInit();
	// Set GLFW to NOT work with OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

void processInput()
{
}

void update()
{
	angleRot += 30.0f * deltaTime;
	glm::mat4 t = glm::rotate(glm::mat4(1.0f), glm::radians(angleRot), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.7f));
	vulkanRenderer.updateModelTransform(modelId, t);
}

void render()
{
	vulkanRenderer.draw();
}

int main()
{
	modelId = 1;
	auto model = importModel("VulkanCourseApp/assets/SeahawkBlender/SeahawkBlender.obj");

	// Initialize window
	initWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	// Create and initialize Vulkan Renderer Instance
	if (vulkanRenderer.init(window) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	for (int i = 0; i < model.size(); i++)
	{
		vulkanRenderer.addToRenderer(modelId, model.size(), model.data(), glm::vec3(0.8f, 0.8f, 0.8f));
	}

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