#version 450

layout(location = 0) in vec4 vColor;

layout(location = 0) out vec4 fColor;

/*layout(binding = 1) uniform Colors
{
	vec4 diffuse;
};*/

void main()
{
	//fColor = diffuse * vColor;
	fColor = vColor;
}
