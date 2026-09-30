#version 140

out vec2 v_TEXCOORD;

void main()
{
    vec4 _30 = vec4((uint(gl_VertexID) == 2u) ? 3.0 : (-1.0), (uint(gl_VertexID) == 0u) ? 3.0 : (-1.0), 1.0, 1.0);
    gl_Position = _30;
    v_TEXCOORD = (_30.xy * vec2(0.5, -0.5)) + vec2(0.5);
}

