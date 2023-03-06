// GLSL vertex shader

#version 430 core

layout(std140) uniform Scene
{
	mat4 vpMatrix;
};

uniform uint instance;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec4 vWorldPos;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vTexCoord;


// VERTEX SHADER

struct Transform
{
	mat4 wMatrix;
};

layout(std430) buffer transforms
{
    Transform transformsArray[];
};

void main()
{
	Transform transform = transformsArray[instance];
	vWorldPos   = transform.wMatrix * vec4(position, 1);
	gl_Position = vpMatrix * vWorldPos;
	vNormal     = (transform.wMatrix * vec4(normal, 0)).xyz;
	vTexCoord   = texCoord;
}

