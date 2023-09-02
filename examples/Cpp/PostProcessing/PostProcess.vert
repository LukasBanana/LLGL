// GLSL post-processor vertex shader

#version 140

#ifdef GL_ES
precision mediump float;
#endif

out vec2 vTexCoord;

/*
This function generates the coordinates for a fullscreen triangle with its vertex IDs:

(-1,+3)
   *
   | \
   |   \
   |     \
   |       \
(-1,+1)    (+1,+1)
   *----------*\
   |          |  \
   |  screen  |    \
   |          |      \
   *----------*--------*
(-1,-1)    (+1,-1)  (+3,-1)
*/
vec4 GetFullscreenTriangleVertex(int id)
{
    return vec4(
        (id == 2 ? 3.0 : -1.0),
        (id == 0 ? 3.0 : -1.0),
        1.0,
        1.0
    );
}

void main()
{
    // Generate coorindate for fullscreen triangle
    gl_Position = GetFullscreenTriangleVertex(gl_VertexID);

    // Get texture-coordinate from vertex position
    vTexCoord = gl_Position.xy * vec2(0.5, -0.5) + 0.5;
}
