// GLSL vertex shader
#version 450 core

layout(std140, binding = 2) uniform Settings
{
	mat4 vpMatrix;
    vec4 viewPos;
    vec4 fogColorAndDensity;
	vec2 animVec;
};

// Per-vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

// Per-instance attributes
layout(location = 2) in vec3 color;
layout(location = 3) in float arrayLayer;
layout(location = 4) in mat4 wMatrix;

// Vertex output to the fragment shader
layout(location = 0) out vec4 vWorldPos;
layout(location = 1) out vec3 vTexCoord;
layout(location = 2) out vec3 vColor;

out gl_PerVertex
{
    vec4 gl_Position;
};

// Vertex shader main function
void main()
{
	vec2 offset = animVec * position.y;
	
	vec4 coord = vec4(
		position.x + offset.x,
		position.y,
		position.z + offset.y,
		1.0
	);
	
    vWorldPos = wMatrix * coord;
	gl_Position = vpMatrix * vWorldPos;
	
	vTexCoord = vec3(texCoord, arrayLayer);
	
	vColor = color;
}
