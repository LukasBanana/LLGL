/*
 * RenderingProfiler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/RenderingProfiler.h>
#include <algorithm>


namespace LLGL
{


void RenderingProfiler::NextProfile(FrameProfile* outputProfile)
{
    /* Copy current counters to the output profile (if set) */
    if (outputProfile)
        *outputProfile = frameProfile;

    /* Clear values */
    frameProfile.Clear();
}

void RenderingProfiler::Accumulate(const FrameProfile& profile)
{
    frameProfile.Accumulate(profile);
}


} // /namespace LLGL



// ================================================================================
