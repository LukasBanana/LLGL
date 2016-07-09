/*
 * Error.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_ERROR_H__
#define __LLGL_ERROR_H__


#include <string>
#include <LLGL/API.h>


namespace LLGL
{


enum class Error
{
    None,
    //...
};


LLGL_EXPORT LLGL_API std::string ToString(Error value);


} // /namespace LLGL


#endif



// ================================================================================
