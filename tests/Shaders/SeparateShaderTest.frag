#version 420 core

in vec4 vColor;

out vec4 fColor;

layout(binding = 0) uniform Settings
{
	vec4 offset;
	vec4 albedo;
};

//layout(binding = 2) uniform sampler2D colorMapA;

void main()
{
    fColor = vColor * albedo;// + texture(colorMapA, vColor.xy);
}
