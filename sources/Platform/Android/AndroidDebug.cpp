/*
 * AndroidDebug.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../Debug.h"
#include <android/log.h>


namespace LLGL
{


LLGL_EXPORT void DebugPuts(const char* text)
{
    (void)__android_log_print(ANDROID_LOG_ERROR, "LLGL", "%s\n", text);
}

LLGL_EXPORT UTF8String DebugStackTrace(unsigned firstStackFrame, unsigned maxNumStackFrames)
{
    return {}; //TODO
}


} // /namespace LLGL



// ================================================================================
