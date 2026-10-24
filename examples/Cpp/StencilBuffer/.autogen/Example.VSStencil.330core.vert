#version 330

layout(std140) uniform SceneView
{
    layout(row_major) mat4 wMatrix;
    layout(row_major) mat4 vpMatrix;
    vec3 lightDir;
    float reflectionFading;
    vec4 diffuse;
};

layout(location = 0) in vec3 POSITION;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    gl_Position = (vec4(POSITION, 1.0) * spvWorkaroundRowMajor(wMatrix)) * spvWorkaroundRowMajor(vpMatrix);
}

