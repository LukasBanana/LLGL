#version 320 es

layout(binding = 1, std140) uniform Settings
{
    layout(row_major) mat4 cMatrix;
    layout(row_major) mat4 vpMatrix;
    layout(row_major) mat4 wMatrix;
    vec2 aspectRatio;
    float mipCount;
    float _pad0;
    vec4 lightDir;
    uint skyboxLayer;
    uint materialLayer;
    uvec2 _pad1;
};

layout(location = 0) out vec4 v_VIEWRAY;

void main()
{
    vec4 _34 = vec4((uint(gl_VertexID) == 2u) ? 3.0 : (-1.0), (uint(gl_VertexID) == 0u) ? 3.0 : (-1.0), 1.0, 1.0);
    gl_Position = _34;
    v_VIEWRAY = vec4(_34.xy * aspectRatio, 1.0, 0.0);
}

