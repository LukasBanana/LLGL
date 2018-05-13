/*
 * Log.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Log.h>
#include <iostream>


namespace LLGL
{

namespace Log
{


struct LogState
{
    LogState() :
        stdOut { &(std::cout) },
        stdErr { &(std::cerr) }
    {
    }

    std::ostream* stdOut = nullptr;
    std::ostream* stdErr = nullptr;
};

static LogState g_logState;


LLGL_EXPORT void SetStdOut(std::ostream& stream)
{
    g_logState.stdOut = &stream;
}

LLGL_EXPORT void SetStdErr(std::ostream& stream)
{
    g_logState.stdErr = &stream;
}

LLGL_EXPORT std::ostream& StdOut()
{
    return *(g_logState.stdOut);
}

LLGL_EXPORT std::ostream& StdErr()
{
    return *(g_logState.stdErr);
}


} // /namespace Log

} // /namespace LLGL



// ================================================================================
