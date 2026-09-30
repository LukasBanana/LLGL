#version 300 es

layout(std140) uniform SceneView
{
    layout(row_major) mat4 cMatrix;
    layout(row_major) mat4 vpMatrix;
    layout(row_major) mat4 wMatrix;
    vec2 aspectRatio;
    float mipCount;
    float _pad0;
    vec4 lightDir;
};

out vec4 v_VIEWRAY;

void main()
{
    vec4 _33 = vec4((uint(gl_VertexID) == 2u) ? 3.0 : (-1.0), (uint(gl_VertexID) == 0u) ? 3.0 : (-1.0), 1.0, 1.0);
    gl_Position = _33;
    v_VIEWRAY = vec4(_33.xy * aspectRatio, 1.0, 0.0);
}

