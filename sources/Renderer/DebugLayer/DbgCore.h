/*
 * DbgCore.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DBG_CORE_H__
#define __LLGL_DBG_CORE_H__


#include <LLGL/RenderingProfiler.h>
#include <LLGL/RenderingDebugger.h>


#define LLGL_DBG_PROFILER_DO(EXPR)  \
    if (profiler_)                  \
        profiler_->EXPR

#define LLGL_DBG_DEBUGGER_DO(STMNT) \
    if (debugger_)                  \
        STMNT

#define LLGL_DBG_ERROR(TYPE, MSG, SOURCE)               \
    if (debugger_)                                      \
        debugger_->PostError((TYPE), (MSG), (SOURCE))

#define LLGL_DBG_WARN(TYPE, MSG, SOURCE)                \
    if (debugger_)                                      \
        debugger_->PostWarning((TYPE), (MSG), (SOURCE))


#endif



// ================================================================================
