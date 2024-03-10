/*
 * AndroidGLCore.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "AndroidGLCore.h"
#include "../../../../Core/MacroUtils.h"


namespace LLGL
{


const char* EGLErrorToString(EGLint errorCode)
{
    switch (errorCode)
    {
        LLGL_CASE_TO_STR( EGL_SUCCESS             );
        LLGL_CASE_TO_STR( EGL_NOT_INITIALIZED     );
        LLGL_CASE_TO_STR( EGL_BAD_ACCESS          );
        LLGL_CASE_TO_STR( EGL_BAD_ALLOC           );
        LLGL_CASE_TO_STR( EGL_BAD_ATTRIBUTE       );
        LLGL_CASE_TO_STR( EGL_BAD_CONTEXT         );
        LLGL_CASE_TO_STR( EGL_BAD_CONFIG          );
        LLGL_CASE_TO_STR( EGL_BAD_CURRENT_SURFACE );
        LLGL_CASE_TO_STR( EGL_BAD_DISPLAY         );
        LLGL_CASE_TO_STR( EGL_BAD_SURFACE         );
        LLGL_CASE_TO_STR( EGL_BAD_MATCH           );
        LLGL_CASE_TO_STR( EGL_BAD_PARAMETER       );
        LLGL_CASE_TO_STR( EGL_BAD_NATIVE_PIXMAP   );
        LLGL_CASE_TO_STR( EGL_BAD_NATIVE_WINDOW   );
        LLGL_CASE_TO_STR( EGL_CONTEXT_LOST        );
    }
    return "";
}

const char* EGLErrorToString()
{
    return EGLErrorToString(eglGetError());
}


} // /namespace LLGL



// ================================================================================
