#include <SDL2/SDL.h>
#include <stdio.h>
#include "model.h"
#include "vkc.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

extern void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
extern void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

int loadModel(Model *model, const char* path, const char* filename)
{
    cgltf_options options = { 0 };
    cgltf_data* cgltfData = NULL;
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "%s/%s", path, filename);

    cgltf_result result = cgltf_parse_file(&options, filePath, &cgltfData);
    if (result != cgltf_result_success)
    {
        SDL_Log("Error loading GLTF model %s: result %d!\n", filePath, result);
        goto loadGltfModel_error;
    }
    cgltf_result result2 = cgltf_load_buffers(&options, cgltfData, filePath);
    if (result2 != cgltf_result_success)
    {
        SDL_Log("Error loading GLTF model %s buffers: result %d!\n", filePath, result2);
        goto loadGltfModel_error;
    }
    snprintf(model->textureImagePath, sizeof(model->textureImagePath), "%s/%s", path, cgltfData->images[0].uri);

    // parse the node tree, find the first model node
    struct nodeStackEntry {
        cgltf_node* node;
        mat4 matrix;
    } nodeStack[32] = { 0 };
    int stackSize = 0;
    if (cgltfData->nodes_count > 0) {
        nodeStack[stackSize].node = &cgltfData->nodes[0];
        glm_mat4_identity(nodeStack[stackSize].matrix);
        stackSize++;
    }
    mat4 transform = GLM_MAT4_IDENTITY_INIT;
    cgltf_mesh* mesh = NULL;
    while (stackSize > 0)
    {
        struct nodeStackEntry* entry = &nodeStack[stackSize - 1];
        cgltf_node* node = entry->node;
        if (node->has_matrix) glm_mat4_mul(node->matrix, entry->matrix, entry->matrix);
        if (node->mesh) {
            mesh = node->mesh;
            glm_mat4_copy(entry->matrix, transform);
            break;
        }
        stackSize--;
        for (int i = 0; i < node->children_count; i++) {
            nodeStack[stackSize].node = node->children[i];
            glm_mat4_copy(entry->matrix, nodeStack[stackSize].matrix);
            stackSize++;
        }
    }

    if (!mesh) {
        SDL_Log("Model %s does not contain a mesh node!\n", filePath);
    }

    cgltf_primitive* primitive = mesh->primitives;
    if (primitive->type != cgltf_primitive_type_triangles) {
        SDL_Log("Expected cgltf_primitive_type_triangles!\n");
        goto loadGltfModel_error;
    }

    int verticesCount = primitive->attributes[0].data->count;
    model->vertices = calloc(verticesCount, sizeof(model->vertices[0]));
    model->verticesSize = verticesCount * sizeof(model->vertices[0]);
    for (int i = 0; i < primitive->attributes_count; i++)
    {
        cgltf_attribute* attribute = &primitive->attributes[i];
        if (attribute->type == cgltf_attribute_type_position) {
            if (attribute->data->stride != 12) {
                SDL_Log("Expected 3*float for position attribute!\n");
                goto loadGltfModel_error;
            }
            const uint8_t* dataBytes = cgltf_buffer_view_data(attribute->data->buffer_view);
            float* dataFloats = (float*)(dataBytes + attribute->data->offset);
            Vertex* vtx = model->vertices;
            for (int i = 0; i < verticesCount; i++) {
                vec4 v = { 0.0f, 0.0f, 0.0f, 1.0f };
                // pre-transform vertex
                v[0] = dataFloats[i * 3 + 0];
                v[1] = dataFloats[i * 3 + 1];
                v[2] = dataFloats[i * 3 + 2];
                glm_mat4_mulv(transform, v, v);

                vtx[i].pos[0] = v[0];
                vtx[i].pos[1] = v[1];
                vtx[i].pos[2] = v[2];

                // do not read color values from model, just assume white for every vertex
                vtx[i].color[1] = 1.0f;
                vtx[i].color[2] = 1.0f;
                vtx[i].color[0] = 1.0f;
            }
        } else if (attribute->type == cgltf_attribute_type_normal) {
            if (attribute->data->stride != 12) {
                SDL_Log("Expected 3*float for normal attribute!\n");
                goto loadGltfModel_error;
            }
            const uint8_t* dataBytes = cgltf_buffer_view_data(attribute->data->buffer_view);
            float* dataFloats = (float*)(dataBytes + attribute->data->offset);
            Vertex* vtx = model->vertices;

            mat3 ntransform;
            glm_mat4_pick3(transform, ntransform);
            glm_mat3_inv(ntransform, ntransform);
            glm_mat3_transpose(ntransform);

            for (int i = 0; i < verticesCount; i++) {
                vec4 v = { 0.0f, 0.0f, 0.0f, 1.0f };
                // pre-transform normal
                v[0] = dataFloats[i * 3 + 0];
                v[1] = dataFloats[i * 3 + 1];
                v[2] = dataFloats[i * 3 + 2];
                glm_mat3_mulv(ntransform, v, v);
                glm_vec3_normalize(v);

                vtx[i].normal[0] = v[0];
                vtx[i].normal[1] = v[1];
                vtx[i].normal[2] = v[2];
            }
        } else if (attribute->type == cgltf_attribute_type_texcoord) {
            if (attribute->data->stride != 8) {
                SDL_Log("Expected 2*float for texcoord attribute!\n");
                goto loadGltfModel_error;
            }
            const uint8_t* dataBytes = cgltf_buffer_view_data(attribute->data->buffer_view);
            float* dataFloats = (float*)(dataBytes + attribute->data->offset);
            Vertex* vtx = model->vertices;
            for (int i = 0; i < verticesCount; i++) {
                vtx[i].texCoord[0] = dataFloats[i * 2 + 0];
                vtx[i].texCoord[1] = dataFloats[i * 2 + 1];
            }
        }
    }
    int indicesCount = primitive->indices->count;
    model->indicesSize = indicesCount * sizeof(model->indices[0]);
    model->indices = calloc(indicesCount, sizeof(model->indices[0]));
    if (primitive->indices->component_type != cgltf_component_type_r_16u) {
        SDL_Log("Expected unsigned 16-bit indices.\n");
        goto loadGltfModel_error;
    }
    uint16_t* indicesData = cgltf_buffer_view_data(primitive->indices->buffer_view);
    for (int i = 0; i < indicesCount; i++)
    {
        model->indices[i] = indicesData[i];
    }
    cgltf_free(cgltfData);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    void* bufferData;

    // vertex buffer
    createBuffer((VkDeviceSize)model->verticesSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        &stagingBuffer, &stagingBufferMemory);
    vkMapMemory(vkc.device, stagingBufferMemory, 0, (VkDeviceSize)model->verticesSize, 0, &bufferData);
    memcpy(bufferData, model->vertices, model->verticesSize);
    vkUnmapMemory(vkc.device, stagingBufferMemory);
    createBuffer((VkDeviceSize)model->verticesSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        &model->vertexBuffer, &model->vertexBufferMemory);
    copyBuffer(stagingBuffer, model->vertexBuffer, (VkDeviceSize)model->verticesSize);
    vkDestroyBuffer(vkc.device, stagingBuffer, NULL);
    vkFreeMemory(vkc.device, stagingBufferMemory, NULL);

    // index buffer
    createBuffer((VkDeviceSize)model->indicesSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        &stagingBuffer, &stagingBufferMemory);
    vkMapMemory(vkc.device, stagingBufferMemory, 0, (VkDeviceSize)model->indicesSize, 0, &bufferData);
    memcpy(bufferData, model->indices, model->indicesSize);
    vkUnmapMemory(vkc.device, stagingBufferMemory);
    createBuffer((VkDeviceSize)model->indicesSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        &model->indexBuffer, &model->indexBufferMemory);
    copyBuffer(stagingBuffer, model->indexBuffer, (VkDeviceSize)model->indicesSize);
    vkDestroyBuffer(vkc.device, stagingBuffer, NULL);
    vkFreeMemory(vkc.device, stagingBufferMemory, NULL);

    return 0;

    loadGltfModel_error:
    cgltf_free(cgltfData);
    free(model->vertices);
    model->vertices = NULL;
    free(model->indices);
    model->indices = NULL;
    SDL_Log("Error loading model %s.\n", filePath);
    return 1;
}

int destroyModel(Model* model)
{
    free(model->vertices);
    model->vertices = NULL;
    free(model->indices);
    model->indices = NULL;

    vkDestroyBuffer(vkc.device, model->indexBuffer, NULL);
    vkFreeMemory(vkc.device, model->indexBufferMemory, NULL);
    vkDestroyBuffer(vkc.device, model->vertexBuffer, NULL);
    vkFreeMemory(vkc.device, model->vertexBufferMemory, NULL);

    return VK_SUCCESS;
}
