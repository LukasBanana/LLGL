#version 150
#extension GL_ARB_viewport_array : enable
layout(triangles) in;
layout(max_vertices = 6, triangle_strip) out;

struct OutputVS
{
    vec4 position;
    vec3 color;
};

in vec3 v_COLOR[3];
out vec3 g_COLOR;

void main()
{
    vec4 _36_unrolled[3];
    for (int i = 0; i < int(3); i++)
    {
        _36_unrolled[i] = gl_in[i].gl_Position;
    }
    OutputVS param_var_inp[3] = OutputVS[](OutputVS(_36_unrolled[0], v_COLOR[0]), OutputVS(_36_unrolled[1], v_COLOR[1]), OutputVS(_36_unrolled[2], v_COLOR[2]));
    for (int _49 = 0; _49 < 3; )
    {
        gl_Position = param_var_inp[_49].position;
        g_COLOR = param_var_inp[_49].color;
        gl_ViewportIndex = int(0u);
        EmitVertex();
        _49++;
        continue;
    }
    EndPrimitive();
    for (int _59 = 0; _59 < 3; )
    {
        vec2 _67 = -param_var_inp[_59].position.xy;
        gl_Position = vec4(_67.x, _67.y, param_var_inp[_59].position.z, param_var_inp[_59].position.w);
        g_COLOR = param_var_inp[_59].color;
        gl_ViewportIndex = int(1u);
        EmitVertex();
        _59++;
        continue;
    }
    EndPrimitive();
}

