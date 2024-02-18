#pragma once
#include "vkc.h"

typedef struct {
	Vertex* vertices;
	int verticesSize;
	uint32_t* indices;
	int indicesSize;
	char textureImagePath[256];

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
} Model;

extern int loadModel(Model* model, const char* path, const char* filename);
extern int destroyModel(Model* model);
