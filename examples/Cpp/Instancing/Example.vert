// GLSL vertex shader
#version 140

layout(std140) uniform Settings
{
	mat4 vpMatrix;
    vec4 viewPos;
    vec4 fogColorAndDensity;
	vec2 animVec;
};

// Per-vertex attributes
in vec3 position;
in vec2 texCoord;

// Per-instance attributes
in vec3 color;
in float arrayLayer;
in mat4 wMatrix;

// Vertex output to the fragment shader
out vec4 vWorldPos;
out vec3 vTexCoord;
out vec3 vColor;

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
