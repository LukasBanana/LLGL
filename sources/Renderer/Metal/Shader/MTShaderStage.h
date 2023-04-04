/*
 * MTShaderStage.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
