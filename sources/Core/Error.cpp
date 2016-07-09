/*
 * Error.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Error.h>


namespace LLGL
{


LLGL_EXPORT LLGL_API std::string ToString(Error value)
{
    switch (value)
    {
        case Error::None:
            return "None";
    }
    return "";
}



} // /namespace LLGL



// ================================================================================
