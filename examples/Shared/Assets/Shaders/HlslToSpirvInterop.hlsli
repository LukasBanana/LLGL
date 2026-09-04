/*
 * HlslToSpirvInterop.hlsli
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#if __spirv__

// Unpacks three float4 matrix rows to mimic D3D's compact matrix memory layout
column_major float4x3 UnpackRowMajor3x4Matrix(float4 row0, float4 row1, float4 row2)
{
    return float4x3(
        float3(row0.x, row0.y, row0.z),
        float3(row0.w, row1.x, row1.y),
        float3(row1.z, row1.w, row2.x),
        float3(row2.y, row2.z, row2.w)
    );
}

#define DECLARE_MAT3x4(MAT) \
    float4 MAT##Row0; \
    float4 MAT##Row1; \
    float4 MAT##Row2

#define MAT3x4_MUL(MAT, VEC) \
    mul(VEC, UnpackRowMajor3x4Matrix(MAT##Row0, MAT##Row1, MAT##Row2))

#else // __spirv__

#define DECLARE_MAT3x4(MAT) \
    float3x4 MAT

#define MAT3x4_MUL(MAT, VEC) \
    mul(MAT, VEC)

#endif // /__spirv__

#if NDC_SPACE_UNIT_CUBE

#define NDC_TO_CLIP_SPACE(VEC) \
    VEC.xyz = (VEC.xyz * float3(0.5, -0.5, 0.5) + 0.5)

#else

#define NDC_TO_CLIP_SPACE(VEC) \
    VEC.xy = (VEC.xy * float2(0.5, -0.5) + 0.5)

#endif

