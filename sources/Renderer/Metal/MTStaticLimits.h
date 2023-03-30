/*
 * MTStaticLimits.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_STATIC_LIMITS_H
#define LLGL_MT_STATIC_LIMITS_H


//TODO: Maybe replace these macros by making "MTResourceHeap::MTShaderStage" a global enum

// Number of Metal shader stages there are. 3 for Metal (vertex, fragment, and kernel).
#define LLGL_MT_NUM_SHADER_STAGES       (3u)

// Internal index for vertex shader stage.
#define LLGL_MT_SHADER_STAGE_VERTEX     (0u)

// Internal index for fragment shader stage.
#define LLGL_MT_SHADER_STAGE_FRAGMENT   (1u)

// Internal index for kernel shader stage.
#define LLGL_MT_SHADER_STAGE_KERNEL     (2u)


#endif



// ================================================================================
