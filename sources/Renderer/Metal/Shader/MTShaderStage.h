/*
 * MTShaderStage.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_SHADER_STAGE_H
#define LLGL_MT_SHADER_STAGE_H


#include <cstdint>


namespace LLGL
{


// Metal shader stage enumeration.
enum MTShaderStage : std::uint32_t
{
    MTShaderStage_Vertex = 0,
    MTShaderStage_Fragment,
    MTShaderStage_Kernel,

    MTShaderStage_Count,
    MTShaderStage_CountPerPSO = 2,
};


} // /namespace LLGL


#endif



// ================================================================================
