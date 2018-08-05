// GLSL shader version 4.50 (for Vulkan)
#version 450

layout(triangles, invocations = 2) in;
layout(triangle_strip, max_vertices = 3) out;

// Geometry input from the vertex shader
layout(location = 0) in vec3 vertexColor[];

// Geometry output color
layout(location = 0) out vec3 geometryColor;

// Geometry shader main function
void main()
{
    vec4 transform[2] = vec4[](vec4(1.0), vec4(-1.0, -1.0, 1.0, 1.0));

	// Write vertices to viewport for current invocation
	for (int i = 0; i < gl_in.length(); ++i)
	{
		geometryColor = vertexColor[i];
		gl_Position = gl_in[i].gl_Position * transform[gl_InvocationID];
        gl_PrimitiveID = gl_PrimitiveIDIn;
		gl_ViewportIndex = gl_InvocationID;
		EmitVertex();
	}
    
	EndPrimitive();
}
