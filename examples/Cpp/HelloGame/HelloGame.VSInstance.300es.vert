#version 300 es

// ESSL shader for HelloGame example
// This file is part of the LLGL project

#ifndef MAX_NUM_INSTANCES
#define MAX_NUM_INSTANCES 64
#endif

precision mediump float;

layout(std140) uniform Scene
{
    mat4    vpMatrix;
    mat4    vpShadowMatrix;
    vec3    lightDir;
    float   shininess;
    vec3    viewPos;
    float   shadowSizeInv;
    vec3    warpCenter;
    float   warpIntensity;
    vec3    bendDir;
    float   ambientItensity;
    vec3    groundTint;
    float   groundScale;
    vec3    lightColor;
    float   warpScaleInv;
};

struct Instance
{
    vec4    wMatrixRow0;
    vec4    wMatrixRow1;
    vec4    wMatrixRow2;
    vec4    color;
};


// VERTEX SHADER INSTANCE

layout(std140) uniform instances
{
    Instance instancesArray[MAX_NUM_INSTANCES];
};

uniform vec3    worldOffset;
uniform float   bendIntensity;
uniform uint    firstInstance;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vTexCoord;
out vec4 vColor;

float WarpIntensityCurve(float d)
{
    return 1.0/(1.0 + d*d*warpScaleInv);
}

vec3 MeshAnimation(vec3 pos)
{
    return pos + bendDir * pos.y * bendIntensity;
}

vec3 WarpPosition(vec3 pos)
{
    vec3 dir = pos - warpCenter;
    float dirLen = length(dir);
    float intensity = WarpIntensityCurve(dirLen);
    return pos + normalize(dir) * intensity * warpIntensity;
}

// Unpacks three vec4 matrix rows to mimic D3D's compact matrix memory layout
mat4x3 UnpackRowMajor3x4Matrix(vec4 row0, vec4 row1, vec4 row2)
{
    return mat4x3(
        vec3(row0.x, row0.y, row0.z),
        vec3(row0.w, row1.x, row1.y),
        vec3(row1.z, row1.w, row2.x),
        vec3(row2.y, row2.z, row2.w)
    );
}

void main()
{
    Instance inst = instancesArray[uint(gl_InstanceID) + firstInstance];
    mat4x3 wMatrix = UnpackRowMajor3x4Matrix(inst.wMatrixRow0, inst.wMatrixRow1, inst.wMatrixRow2);
    vec4 modelPos = vec4(MeshAnimation(position), 1);
    vWorldPos   = WarpPosition((wMatrix * modelPos) + worldOffset);
    gl_Position = vpMatrix * vec4(vWorldPos, 1);
    vNormal     = wMatrix * vec4(normal, 0);
    vTexCoord   = texCoord;
    vColor      = inst.color;
}
