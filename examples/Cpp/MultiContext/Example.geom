// GLSL shader version 1.50 (for OpenGL 3.2)
#version 150 core
#extension GL_ARB_viewport_array : enable

layout(triangles) in;
layout(triangle_strip, max_vertices = 6) out;

// Geometry input from the vertex shader
in vec3 vertexColor[];

// Geometry output color
out vec3 geometryColor;

// Geometry shader main function
void main()
{
	// Write vertices to 1st viewport
	for (int i = 0; i < gl_in.length(); ++i)
	{
		geometryColor = vertexColor[i];
		gl_Position = gl_in[i].gl_Position;
		gl_ViewportIndex = 0;
		EmitVertex();
	}
	
	EndPrimitive();
	
	// Write vertices to 2nd viewport
	for (int i = 0; i < gl_in.length(); ++i)
	{
		geometryColor = vertexColor[i];
		vec4 pos = gl_in[i].gl_Position;
		pos.xy = -pos.xy;
		gl_Position = pos;
		gl_ViewportIndex = 1;
		EmitVertex();
	}
	
	EndPrimitive();
}
