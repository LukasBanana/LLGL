/*
 * RenderingProfiler.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDERING_PROFILER_H
#define LLGL_RENDERING_PROFILER_H


#include <LLGL/RenderingDebugger.h>
#include <LLGL/Deprecated.h>


namespace LLGL
{


//! \deprecated Since 0.04b; Use RenderingDebugger instead!
class LLGL_DEPRECATED("LLGL::RenderingProfiler is deprecated since 0.04b; Use LLGL::RenderingDebugger instead!", "RenderingDebugger") RenderingProfiler
{

    public:

        //! \deprecated Since 0.04b; Use RenderingDebugger::FlushProfile instead!
        inline void NextProfile(FrameProfile* outputProfile = nullptr) {}

        //! \deprecated Since 0.04b; Use RenderingDebugger::RecordProfile instead!
        inline void Accumulate(const FrameProfile& profile) {}

    public:

        //! \deprecated Since 0.04b; Use RenderingDebugger::FlushProfile instead!
        FrameProfile    frameProfile;

        //! \deprecated Since 0.04b; Use RenderingDebugger::SetTimeRecording instead!
        bool            timeRecordingEnabled    = false;

};


} // /namespace LLGL


#endif



// ================================================================================
