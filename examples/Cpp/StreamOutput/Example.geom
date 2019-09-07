// GLSL geometry shader
#version 150 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 9) out;

void main()
{
	// Generate 3 instances
	for (int i = 0; i < 3; ++i)
	{
		// Generate 3 vertices for each triangle
		for (int j = 0; j < 3; ++j)
		{
			vec4 pos = gl_in[j].gl_Position;
			pos.x += (float(i) - 1.0)*4.0;
			pos.w = 8.0;
			gl_Position = pos;
			EmitVertex();
		}
		
		EndPrimitive();
	}
}
