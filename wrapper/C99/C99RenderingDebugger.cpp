/*
 * C99RenderingDebugger.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/RenderingDebugger.h>
#include <LLGL-C/RenderingDebugger.h>
#include <LLGL/Utils/ForRange.h>
#include "C99Internal.h"
#include <cstring>


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT LLGLRenderingDebugger llglAllocRenderingDebugger()
{
    return LLGLRenderingDebugger{ new RenderingDebugger };
}

LLGL_C_EXPORT void llglFreeRenderingDebugger(LLGLRenderingDebugger debugger)
{
    delete LLGL_PTR(RenderingDebugger, debugger);
}

LLGL_C_EXPORT void llglSetDebuggerTimeRecording(LLGLRenderingDebugger debugger, bool enabled)
{
    LLGL_PTR(RenderingDebugger, debugger)->SetTimeRecording(enabled);
}

LLGL_C_EXPORT bool llglGetDebuggerTimeRecording(LLGLRenderingDebugger debugger)
{
    return LLGL_PTR(RenderingDebugger, debugger)->GetTimeRecording();
}

static void ConvertC99ProfileTimeRecord(LLGLProfileTimeRecord& dst, const ProfileTimeRecord& src)
{
    dst.annotation      = src.annotation.c_str();
    dst.cpuTicksStart   = src.cpuTicksStart;
    dst.cpuTicksEnd     = src.cpuTicksEnd;
    dst.elapsedTime     = src.elapsedTime;
}

LLGL_C_EXPORT void llglFlushDebuggerProfile(LLGLRenderingDebugger debugger, LLGLFrameProfile* outFrameProfile)
{
    LLGL_ASSERT_PTR(outFrameProfile);

    static thread_local FrameProfile internalFrameProfile;
    static thread_local std::vector<LLGLProfileTimeRecord> internalProfileTimeRecords;
    LLGL_PTR(RenderingDebugger, debugger)->FlushProfile(&internalFrameProfile);

    static_assert(
        sizeof(LLGLProfileCommandQueueRecord) == sizeof(ProfileCommandQueueRecord),
        "LLGLProfileCommandQueueRecord and LLGL::ProfileCommandQueueRecord expected to be the same size"
    );
    std::memcpy(&(outFrameProfile->commandQueueRecord), &(internalFrameProfile.commandQueueRecord), sizeof(LLGLProfileCommandQueueRecord));

    static_assert(
        sizeof(LLGLProfileCommandBufferRecord) == sizeof(ProfileCommandBufferRecord),
        "LLGLProfileCommandBufferRecord and LLGL::ProfileCommandBufferRecord expected to be the same size"
    );
    std::memcpy(&(outFrameProfile->commandBufferRecord), &(internalFrameProfile.commandBufferRecord), sizeof(LLGLProfileCommandBufferRecord));

    internalProfileTimeRecords.resize(internalFrameProfile.timeRecords.size());
    for_range(i, internalFrameProfile.timeRecords.size())
        ConvertC99ProfileTimeRecord(internalProfileTimeRecords[i], internalFrameProfile.timeRecords[i]);

    outFrameProfile->numTimeRecords = internalProfileTimeRecords.size();
    outFrameProfile->timeRecords = internalProfileTimeRecords.data();
}


// } /namespace LLGL



// ================================================================================
