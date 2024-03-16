#version 450

layout(binding = 0) uniform PostProcParams {
    vec2 fbSize;
    float time;
} ubo;
layout (binding = 1) uniform sampler2D samplerColor;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    outFragColor = texture(samplerColor, inUV);
}
