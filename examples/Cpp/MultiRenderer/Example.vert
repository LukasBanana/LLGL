// GLSL Vertex Shader "VS"
// Generated by XShaderCompiler
// 28/12/2016 12:38:40

#version 150

in vec3 position;
in vec3 normal;
in vec2 texCoord;

layout(std140) uniform Matrices
{
    mat4 wvpMatrix;
    mat4 wMatrix;
};

out OutputVS
{
    vec3 normal;
    vec2 texCoord;
}
outp;

void main()
{
    vec4 xsc_position;
    xsc_position = (wvpMatrix * vec4(position, 1));
    outp.normal = normalize((wMatrix * vec4(normal, 0)).xyz);
    outp.texCoord = texCoord;
    gl_Position = xsc_position;
}

