#version 420 core

in vec2 position;
in vec4 color;

out vec4 vColor;

layout(binding = 0) uniform Settings
{
	vec4 offset;
	vec4 albedo;
};

//layout(binding = 1) uniform sampler2D colorMapB;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
	gl_Position = vec4(position, 0, 1) + offset;
	vColor = vec4(color);// + texture(colorMapB, offset.xy);
}
