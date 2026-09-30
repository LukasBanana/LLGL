#version 450

layout(binding = 2, std140) uniform SceneState
{
    float time;
    uint numSceneObjects;
    float aspectRatio;
};

layout(location = 0) in vec2 COORD;
layout(location = 1) in vec4 COLOR;
layout(location = 2) in mat2 ROTATION;
layout(location = 4) in vec2 POSITION;
layout(location = 0) out vec4 v_COLOR;

void main()
{
    gl_Position = vec4(((ROTATION * COORD) + POSITION) * vec2(aspectRatio, 1.0), 0.0, 1.0);
    v_COLOR = COLOR;
}

