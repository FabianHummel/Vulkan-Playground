#include <SDL2/SDL.h>
#include <stdio.h>
#include "model.h"
#include "vkc.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include "stb_image.h"
extern void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
extern void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
extern void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
extern void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
extern void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
    VkMemoryPropertyFlags memoryProperties, VkImage* image, VkDeviceMemory* imageMemory);
extern VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

void createMaterialDescriptorSet(Model *model, Material *material)
{
    VkDescriptorSetAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = vkc.descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &vkc.descriptorSetLayouts.textured;

    if (vkAllocateDescriptorSets(vkc.device, &allocInfo, &material->descriptorSet) != VK_SUCCESS) {
        SDL_Log("failed to allocate descriptor set for material!");
        return;
    }

    {
        const int MAX_DESCRIPTOR_WRITES = 16;
        VkWriteDescriptorSet* descriptorWrites = calloc(MAX_DESCRIPTOR_WRITES, sizeof(VkWriteDescriptorSet));
        VkDescriptorBufferInfo bufferInfo = { 0 };
        bufferInfo.buffer = vkc.uniformBuffers.vsShared.buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);
        int i = 0;
        descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet = material->descriptorSet;
        descriptorWrites[i].dstBinding = 0;
        descriptorWrites[i].dstArrayElement = 0;
        descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[i].descriptorCount = 1;
        descriptorWrites[i].pBufferInfo = &bufferInfo;
        i++;

#define TEXTURE(idx) idx >= 0 ? &model->textures[idx] : NULL;
        Texture* baseColorTexture = TEXTURE(material->baseColorTextureIdx);
        Texture* normalTexture = TEXTURE(material->normalTextureIdx);
        Texture* metallicRoughnessTexture = TEXTURE(material->metallicRoughnessTextureIdx);
        VkDescriptorImageInfo imageInfo[4] = { 0 };
#undef TEXTURE
        for (int binding = 1; binding <= 3; binding++) {
            if (binding == 1) {
                if (baseColorTexture) imageInfo[i].imageView = baseColorTexture->imageView;
                else imageInfo[i].imageView = model->colorDefaultTexture.imageView;
            }
            if (binding == 2) {
                if (normalTexture) imageInfo[i].imageView = normalTexture->imageView;
                else imageInfo[i].imageView = model->normalDefaultTexture.imageView;
            }
            if (binding == 3) {
                if (metallicRoughnessTexture) imageInfo[i].imageView = metallicRoughnessTexture->imageView;
                else imageInfo[i].imageView = model->metallicRoughnessDefaultTexture.imageView;
            }
            imageInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo[i].sampler = vkc.textureSampler;
            descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[i].dstSet = material->descriptorSet;
            descriptorWrites[i].dstBinding = binding;
            descriptorWrites[i].dstArrayElement = 0;
            descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[i].descriptorCount = 1;
            descriptorWrites[i].pImageInfo = &imageInfo[i];
            i++;
        }

        size_t descriptorWritesCount = i;
        if (descriptorWritesCount > MAX_DESCRIPTOR_WRITES) {
            SDL_Log("Too many descriptor writes!\n");
        }
        vkUpdateDescriptorSets(vkc.device, (uint32_t)descriptorWritesCount, descriptorWrites, 0, NULL);
        free(descriptorWrites);
    }
}

void createModelTextureImage(Texture *texture)
{
    SDL_Log("Loading %s.\n", texture->imagePath);
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(texture->imagePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        SDL_Log("failed to load texture image %s!\n", texture->imagePath);
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
    void* data;
    vkMapMemory(vkc.device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, (size_t)imageSize);
    vkUnmapMemory(vkc.device, stagingBufferMemory);
    stbi_image_free(pixels);

    createImage((uint32_t)texWidth, (uint32_t)texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &texture->image, &texture->deviceMemory);

    transitionImageLayout(texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, texture->image, (uint32_t)texWidth, (uint32_t)texHeight);
    transitionImageLayout(texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(vkc.device, stagingBuffer, NULL);
    vkFreeMemory(vkc.device, stagingBufferMemory, NULL);

    texture->imageView = createImageView(texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

int loadCgltfPrimitive(Model* model, Primitive *primitive, cgltf_data* cgltfData, cgltf_mesh *mesh, int primitiveIdx)
{
    cgltf_primitive* cgltfPrimitive = &mesh->primitives[primitiveIdx];
    if (cgltfPrimitive->type != cgltf_primitive_type_triangles) {
        SDL_Log("Primitive %d: Expected cgltf_primitive_type_triangles!\n", primitiveIdx);
        return 1;
    }

    if (cgltfPrimitive->material == NULL) {
        SDL_Log("Primitive %d: No material specified!\n", primitiveIdx);
    }

    primitive->materialIdx = cgltfPrimitive->material - cgltfData->materials;

    int verticesCount = cgltfPrimitive->attributes[0].data->count;

    primitive->vertices = calloc(verticesCount, sizeof(primitive->vertices[0]));
    primitive->verticesSize = verticesCount * sizeof(primitive->vertices[0]);
    for (int i = 0; i < cgltfPrimitive->attributes_count; i++)
    {
        cgltf_attribute* attribute = &cgltfPrimitive->attributes[i];
        if (attribute->type == cgltf_attribute_type_position) {
            if (attribute->data->stride != 12) {
                SDL_Log("Expected 3*float for position attribute!\n");
                return 1;
            }
            const uint8_t* dataBytes = cgltf_buffer_view_data(attribute->data->buffer_view);
            float* dataFloats = (float*)(dataBytes + attribute->data->offset);
            Vertex* vtx = primitive->vertices;
            for (int i = 0; i < verticesCount; i++) {
                vec4 v = { 0.0f, 0.0f, 0.0f, 1.0f };
                // pre-transform vertex
                v[0] = dataFloats[i * 3 + 0];
                v[1] = dataFloats[i * 3 + 1];
                v[2] = dataFloats[i * 3 + 2];
                glm_mat4_mulv(model->transform, v, v);

                vtx[i].pos[0] = v[0];
                vtx[i].pos[1] = v[1];
                vtx[i].pos[2] = v[2];

                // do not read color values from model, just assume white for every vertex
                vtx[i].color[1] = 1.0f;
                vtx[i].color[2] = 1.0f;
                vtx[i].color[0] = 1.0f;
            }
        }
        else if (attribute->type == cgltf_attribute_type_normal) {
            if (attribute->data->stride != 12) {
                SDL_Log("Expected 3*float for normal attribute!\n");
                return 1;
            }
            const uint8_t* dataBytes = cgltf_buffer_view_data(attribute->data->buffer_view);
            float* dataFloats = (float*)(dataBytes + attribute->data->offset);
            Vertex* vtx = primitive->vertices;

            mat3 ntransform;
            glm_mat4_pick3(model->transform, ntransform);
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
        }
        else if (attribute->type == cgltf_attribute_type_tangent) {
            if (attribute->data->stride != 16) {
                SDL_Log("Expected 4*float for tangent attribute!\n");
                return 1;
            }
            const uint8_t* dataBytes = cgltf_buffer_view_data(attribute->data->buffer_view);
            float* dataFloats = (float*)(dataBytes + attribute->data->offset);
            Vertex* vtx = primitive->vertices;

            for (int i = 0; i < verticesCount; i++) {
                vtx[i].tangent[0] = dataFloats[i * 4 + 0];
                vtx[i].tangent[1] = dataFloats[i * 4 + 1];
                vtx[i].tangent[2] = dataFloats[i * 4 + 2];
                vtx[i].tangent[3] = dataFloats[i * 4 + 3];
            }
        }
        else if (attribute->type == cgltf_attribute_type_texcoord) {
            if (attribute->data->stride != 8) {
                SDL_Log("Expected 2*float for texcoord attribute!\n");
                return 1;
            }
            const uint8_t* dataBytes = cgltf_buffer_view_data(attribute->data->buffer_view);
            float* dataFloats = (float*)(dataBytes + attribute->data->offset);
            Vertex* vtx = primitive->vertices;
            for (int i = 0; i < verticesCount; i++) {
                vtx[i].texCoord[0] = dataFloats[i * 2 + 0];
                vtx[i].texCoord[1] = dataFloats[i * 2 + 1];
            }
        }
    }
    int indicesCount = cgltfPrimitive->indices->count;
    primitive->indicesSize = indicesCount * sizeof(primitive->indices[0]);
    primitive->indices = calloc(indicesCount, sizeof(primitive->indices[0]));
    if (cgltfPrimitive->indices->component_type != cgltf_component_type_r_16u) {
        SDL_Log("Expected unsigned 16-bit indices.\n");
        return 1;
    }
    const uint8_t* indicesDataBytes = cgltf_buffer_view_data(cgltfPrimitive->indices->buffer_view);
    uint16_t* indicesData = (uint16_t*)(indicesDataBytes + cgltfPrimitive->indices->offset);
    for (int i = 0; i < indicesCount; i++)
    {
        primitive->indices[i] = indicesData[i];
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    void* bufferData;

    // vertex buffer
    createBuffer((VkDeviceSize)primitive->verticesSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);
    vkMapMemory(vkc.device, stagingBufferMemory, 0, (VkDeviceSize)primitive->verticesSize, 0, &bufferData);
    memcpy(bufferData, primitive->vertices, primitive->verticesSize);
    vkUnmapMemory(vkc.device, stagingBufferMemory);
    createBuffer((VkDeviceSize)primitive->verticesSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &primitive->vertexBuffer, &primitive->vertexBufferMemory);
    copyBuffer(stagingBuffer, primitive->vertexBuffer, (VkDeviceSize)primitive->verticesSize);
    vkDestroyBuffer(vkc.device, stagingBuffer, NULL);
    vkFreeMemory(vkc.device, stagingBufferMemory, NULL);

    // index buffer
    createBuffer((VkDeviceSize)primitive->indicesSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);
    vkMapMemory(vkc.device, stagingBufferMemory, 0, (VkDeviceSize)primitive->indicesSize, 0, &bufferData);
    memcpy(bufferData, primitive->indices, primitive->indicesSize);
    vkUnmapMemory(vkc.device, stagingBufferMemory);
    createBuffer((VkDeviceSize)primitive->indicesSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &primitive->indexBuffer, &primitive->indexBufferMemory);
    copyBuffer(stagingBuffer, primitive->indexBuffer, (VkDeviceSize)primitive->indicesSize);
    vkDestroyBuffer(vkc.device, stagingBuffer, NULL);
    vkFreeMemory(vkc.device, stagingBufferMemory, NULL);

    return 0;
}

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

    model->texturesCount = cgltfData->textures_count;
    model->textures = calloc(model->texturesCount, sizeof(Texture));
    for (int i = 0; i < model->texturesCount; i++)
    {
        Texture* texture = &model->textures[i];
        snprintf(texture->imagePath, sizeof(texture->imagePath), "%s/%s", path, cgltfData->textures[i].image->uri);
        createModelTextureImage(texture);
    }

    snprintf(model->colorDefaultTexture.imagePath, sizeof(model->colorDefaultTexture.imagePath), "/Users/fabian/Documents/Schule/4AHIF/FRSEC/vulkan/vulkan/textures/white.png");
    createModelTextureImage(&model->colorDefaultTexture);
    snprintf(model->normalDefaultTexture.imagePath, sizeof(model->normalDefaultTexture.imagePath), "/Users/fabian/Documents/Schule/4AHIF/FRSEC/vulkan/vulkan/textures/normal-null.png");
    createModelTextureImage(&model->normalDefaultTexture);
    snprintf(model->metallicRoughnessDefaultTexture.imagePath, sizeof(model->metallicRoughnessDefaultTexture.imagePath), "/Users/fabian/Documents/Schule/4AHIF/FRSEC/vulkan/vulkan/textures/metallicroughness-null.png");
    createModelTextureImage(&model->metallicRoughnessDefaultTexture);

    model->materialsCount = cgltfData->materials_count;
    model->materials = calloc(model->materialsCount, sizeof(Material));
#define TEXTURE_IDX(tx) tx.texture ? (tx.texture - cgltfData->textures) : -1;
    for (int i = 0; i < model->materialsCount; i++)
    {
        Material* material = &model->materials[i];
        cgltf_material* gltfMaterial = &cgltfData->materials[i];
        glm_vec4_copy(gltfMaterial->pbr_metallic_roughness.base_color_factor, material->baseColorFactor);
        material->baseColorTextureIdx = TEXTURE_IDX(gltfMaterial->pbr_metallic_roughness.base_color_texture);
        material->normalTextureIdx = TEXTURE_IDX(gltfMaterial->normal_texture);
        material->metallicRoughnessTextureIdx = TEXTURE_IDX(gltfMaterial->pbr_metallic_roughness.metallic_roughness_texture);
        createMaterialDescriptorSet(model, material);
    }
#undef TEXTURE_IDX

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
    glm_mat4_identity(model->transform);
    cgltf_mesh* mesh = NULL;
    while (stackSize > 0)
    {
        struct nodeStackEntry* entry = &nodeStack[stackSize - 1];
        cgltf_node* node = entry->node;
        if (node->has_matrix) glm_mat4_mul(node->matrix, entry->matrix, entry->matrix);
        if (node->mesh) {
            mesh = node->mesh;
            glm_mat4_copy(entry->matrix, model->transform);
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
        goto loadGltfModel_error;
    }

    model->primitivesCount = mesh->primitives_count;
    model->primitives = calloc(model->primitivesCount, sizeof(Primitive));

    for (int i = 0; i < model->primitivesCount; i++)
    {
        if (loadCgltfPrimitive(model, &model->primitives[i], cgltfData, mesh, i) != 0) {
            goto loadGltfModel_error;
        }
    }

    
    cgltf_free(cgltfData);
    return 0;

loadGltfModel_error:
    cgltf_free(cgltfData);
    destroyModel(model);
    SDL_Log("Error loading model %s.\n", filePath);
    return 1;
}

void destroyTexture(Texture* texture)
{
    vkFreeMemory(vkc.device, texture->deviceMemory, NULL);
    vkDestroyImage(vkc.device, texture->image, NULL);
    vkDestroyImageView(vkc.device, texture->imageView, NULL);
}

int destroyModel(Model* model)
{
    for (int i = 0; i < model->texturesCount; i++)
    {
        destroyTexture(&model->textures[i]);
    }
    destroyTexture(&model->colorDefaultTexture);
    destroyTexture(&model->normalDefaultTexture);
    destroyTexture(&model->metallicRoughnessDefaultTexture);
    free(model->textures);
    model->textures = NULL;
    free(model->materials);
    model->materials = NULL;

    for (int i = 0; i < model->primitivesCount; i++)
    {
        vkDestroyBuffer(vkc.device, model->primitives[i].indexBuffer, NULL);
        vkFreeMemory(vkc.device, model->primitives[i].indexBufferMemory, NULL);
        vkDestroyBuffer(vkc.device, model->primitives[i].vertexBuffer, NULL);
        vkFreeMemory(vkc.device, model->primitives[i].vertexBufferMemory, NULL);
    }
    free(model->primitives);
    model->primitives = NULL;

    return VK_SUCCESS;
}

void renderPrimitive(Model *model, Primitive* primitive, VkCommandBuffer commandBuffer)
{
    VkBuffer vertexBuffers[] = { primitive->vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, primitive->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    Material* material = &model->materials[primitive->materialIdx];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkc.pipelineLayouts.shadedOffscreen, 0, 1, &material->descriptorSet, 0, NULL);
    vkCmdDrawIndexed(commandBuffer, primitive->indicesSize / sizeof(primitive->indices[0]), 1, 0, 0, 0);
}

void renderModel(Model* model, VkCommandBuffer commandBuffer)
{
    for (int i = 0; i < model->primitivesCount; i++)
    {
        renderPrimitive(model, &model->primitives[i], commandBuffer);
    }
}
