// PBR GLSL Skybox Vertex Shader

#version 410 core

uniform Settings
{
    mat4    cMatrix;
    mat4    vpMatrix;
    mat4    wMatrix;
    vec2    aspectRatio;
    float   mipCount;
    float   _pad0;
    vec4    lightDir;
    uint    skyboxLayer;
    uint    materialLayer;
    uvec2   _pad1;
};

// SKYBOX SHADER

out VSkyOut
{
    vec4 viewRay;
}
outp;

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

    // Generate view ray by vertex coordinate
    outp.viewRay = vec4(gl_Position.xy * aspectRatio, 1, 0);
}


