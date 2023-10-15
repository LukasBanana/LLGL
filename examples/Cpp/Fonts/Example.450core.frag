// GLSL model fragment shader

#version 450 core

layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec4 vColor;

layout(location = 0) out vec4 fragColor;

layout(binding = 2) uniform texture2D glyphTexture;
layout(binding = 3) uniform sampler linearSampler;

void main()
{
    fragColor = vec4(vColor.rgb, vColor.a * texture(sampler2D(glyphTexture, linearSampler), vTexCoord).a);
}
