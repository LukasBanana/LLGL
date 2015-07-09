/*
 * Error.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <details/Error.h>


namespace LLGL
{


LLGL_EXPORT LLGL_API std::string ToString(Error value)
{
    switch (value)
    {
        case Error::None:
            return "None";
        case Error::TextureCreationFailed:
            return "Texture Creation Failed";
        case Error::FramebufferCreationFailed:
            return "Framebuffer Creation Failed";
    }
    return "";
}



} // /namespace LLGL



// ================================================================================
