// GLSL texturing shader
#version 450 core

layout(binding = 0) uniform texture2D tex;
layout(binding = 1) uniform sampler texSampler;

// Fragment input from the vertex shader
layout(location = 0) in vec2 vTexCoord;

// Fragment output color
layout(location = 0) out vec4 fragColor;

// Fragment shader main function
void main()
{
	fragColor = texture(sampler2D(tex, texSampler), vTexCoord);
}
