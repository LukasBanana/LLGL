/*
 * GenerateMips.hlsli
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GENERATE_MIPS_H
#define LLGL_GENERATE_MIPS_H


/* Separate color channels into different groupshared arrays for better cache utilization */
groupshared float sharedColorR[64];
groupshared float sharedColorG[64];
groupshared float sharedColorB[64];
groupshared float sharedColorA[64];

/* Utility functions */
void StoreColor(uint idx, float4 color)
{
    sharedColorR[idx] = color.r;
    sharedColorG[idx] = color.g;
    sharedColorB[idx] = color.b;
    sharedColorA[idx] = color.a;
}

float4 LoadColor(uint idx)
{
    return float4(
        sharedColorR[idx],
        sharedColorG[idx],
        sharedColorB[idx],
        sharedColorA[idx]
    );
}

#ifdef LINEAR_TO_SRGB

float3 LinearToSRGB(float3 linearColor)
{
    /* Use approximation for sRGB curve */
    return (linearColor < 0.0031308 ? 12.92 * linearColor : 1.13005 * sqrt(abs(linearColor - 0.00228)) - 0.13448 * linearColor + 0.005719);
}

#endif // /LINEAR_TO_SRGB

float4 PackLinearColor(float4 linearColor)
{
    #ifdef LINEAR_TO_SRGB
    return float4(LinearToSRGB(linearColor.rgb), linearColor.a);
    #else
    return linearColor;
    #endif // /LINEAR_TO_SRGB
}


#endif
