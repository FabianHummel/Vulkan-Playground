#version 450

layout(binding = 0) uniform PostProcParams {
    vec2 fbSize;
    float time;
} ubo;
layout (binding = 1) uniform sampler2D samplerColor;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

vec4 posterize(in vec4 inputColor)
{
    float gamma = 1.3f;
    float numColors = 20.0f;


    vec3 c = inputColor.rgb;
    c = pow(c, vec3(gamma, gamma, gamma));
    c = c * numColors;
    c = floor(c);
    c = c / numColors;
    c = pow(c, vec3(1.0/gamma));

    return vec4(c, inputColor.a);
}

float warp = 1.0; // simulate curvature of CRT monitor
float scan = 2.0; // simulate darkness between scanlines

vec4 crt(in vec4 inputColor)
{
    vec2 uv = inUV;
    // squared distance from center
    vec2 dc = abs(0.5-uv);
    dc *= dc;
    
    // warp the fragment coordinates
    uv.x -= 0.5; uv.x *= 1.0+(dc.y*(0.3*warp)); uv.x += 0.5;
    uv.y -= 0.5; uv.y *= 1.0+(dc.x*(0.4*warp)); uv.y += 0.5;

    // sample inside boundaries, otherwise set to black
    if (uv.y > 1.0 || uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0)
    {
        return vec4(vec3(0.02), 1.0);
    }
    else
    {
        // determine if we are drawing in a scanline
        float apply = abs(sin(inUV.y * ubo.fbSize.y)*0.5*scan);
        // sample the texture
        return vec4(mix(inputColor.rgb, vec3(0.0), apply), 1.0);
    }
}

vec2 oneTexel = inUV / ubo.fbSize;

vec4 outline()
{
    vec4 center = texture(samplerColor, inUV);
    vec4 up     = texture(samplerColor, inUV + vec2(        0.0, -oneTexel.y));
    vec4 down   = texture(samplerColor, inUV + vec2( oneTexel.x,         0.0));
    vec4 left   = texture(samplerColor, inUV + vec2(-oneTexel.x,         0.0));
    vec4 right  = texture(samplerColor, inUV + vec2(        0.0,  oneTexel.y));
    vec4 uDiff = center - up;
    vec4 dDiff = center - down;
    vec4 lDiff = center - left;
    vec4 rDiff = center - right;
    vec4 sum = uDiff + dDiff + lDiff + rDiff;
    return vec4(vec3((0.299 *sum.r) + (0.587 * sum.g) + (0.114 * sum.b)), 1.0);
}

void main()
{
    // outFragColor = texture(samplerColor, inUV + sin(mod(inUV+ubo.time, 1.0)));
    // outFragColor = texture(samplerColor, inUV + vec2(sin(inUV.x + ubo.time), cos(inUV.y + ubo.time)));
    outFragColor = texture(samplerColor, vec2(
                                              cos(ubo.time) * inUV.x - sin(ubo.time) * inUV.y,
                                              sin(ubo.time) * inUV.x + cos(ubo.time) * inUV.y));

    outFragColor = posterize(outFragColor);
    outFragColor = max(outFragColor, outline() * 10);
    outFragColor = crt(outFragColor);
    outFragColor = outFragColor;
}
