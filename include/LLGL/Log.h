/*
 * Log.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_LOG_H
#define LLGL_LOG_H


#include "Export.h"
#include <ostream>


namespace LLGL
{

namespace Log
{


//! Sets the standard output stream. By default std::cout.
LLGL_EXPORT void SetStdOut(std::ostream& stream);

//! Sets the standard output stream for error and warning messages. By default std::cerr.
LLGL_EXPORT void SetStdErr(std::ostream& stream);

//! Returns the standard output stream.
LLGL_EXPORT std::ostream& StdOut();

//! Returns the standard output stream for error and warning messages.
LLGL_EXPORT std::ostream& StdErr();


} // /namespace Log

} // /namespace LLGL


#endif



// ================================================================================
