#version 450

struct type_PushConstant_Scene_t
{
    mat4 projection;
    vec2 glyphAtlasInvSize;
};

uniform type_PushConstant_Scene_t scene;

layout(location = 0) in ivec2 POSITION;
layout(location = 1) in ivec2 TEXCOORD;
layout(location = 2) in vec4 COLOR;
layout(location = 0) out vec2 v_TEXCOORD;
layout(location = 1) out vec4 v_COLOR;

void main()
{
    gl_Position = scene.projection * vec4(float(POSITION.x), float(POSITION.y), 0.0, 1.0);
    v_TEXCOORD = scene.glyphAtlasInvSize * vec2(float(TEXCOORD.x), float(TEXCOORD.y));
    v_COLOR = COLOR;
}

