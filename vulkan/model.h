#pragma once
#include "vkc.h"

typedef struct {
    char imagePath[256];
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory deviceMemory;
} Texture;

typedef struct {
    vec4 baseColorFactor;
    int baseColorTextureIdx;
    int normalTextureIdx;
    int metallicRoughnessTextureIdx;
    VkDescriptorSet descriptorSet;
} Material;

typedef struct {
    Vertex* vertices;
    int verticesSize;
    uint32_t* indices;
    int indicesSize;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    int materialIdx;
} Primitive;

typedef struct {
    Texture* textures;
    int texturesCount;
    Texture colorDefaultTexture;
    Texture normalDefaultTexture;
    Texture metallicRoughnessDefaultTexture;
    Material* materials;
    int materialsCount;
    Primitive* primitives;
    int primitivesCount;
    mat4 transform;
} Model;

extern int loadModel(Model* model, const char* path, const char* filename);
extern void renderModel(Model* model, VkCommandBuffer commandBuffer);
extern int destroyModel(Model* model);
