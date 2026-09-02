#version 140

struct type_PushConstant_Scene_t
{
    mat4 projection;
    vec2 glyphAtlasInvSize;
};

uniform type_PushConstant_Scene_t scene;

in ivec2 POSITION;
in ivec2 TEXCOORD;
in vec4 COLOR;
out vec2 v_TEXCOORD;
out vec4 v_COLOR;

void main()
{
    gl_Position = scene.projection * vec4(float(POSITION.x), float(POSITION.y), 0.0, 1.0);
    v_TEXCOORD = scene.glyphAtlasInvSize * vec2(float(TEXCOORD.x), float(TEXCOORD.y));
    v_COLOR = COLOR;
}

