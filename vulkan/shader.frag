#version 450

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D normalSampler;
layout(binding = 3) uniform sampler2D metallicRoughnessSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragLightDir;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 ambient = vec3(0.1);
    vec3 diffuse = vec3(max(dot(fragLightDir, fragNormal), 0.0));
    vec3 lightColor = ambient + diffuse;
    vec4 normal = texture(normalSampler, fragTexCoord);
    vec4 metallicRoughness = texture(metallicRoughnessSampler, fragTexCoord);
    outColor = vec4(lightColor * fragColor * texture(texSampler, fragTexCoord).rgb, 1.0);
}
