#version 450

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vTexCoord;

layout(location = 0) out vec4 fColor;

layout(binding = 5) uniform Colors
{
	vec4 diffuse;
};

layout(binding = 3) uniform sampler texSampler;
layout(binding = 4) uniform texture2D tex;

void main()
{
	fColor = diffuse * vColor * texture(sampler2D(tex, texSampler), vTexCoord);
	fColor = mix(vec4(1, 1, 1, 1), fColor, fColor.a);
	fColor.a = 1.0;
}
