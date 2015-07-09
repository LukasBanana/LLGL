/*
 * Texture2DDesc.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_TEXTURE2D_DESC_H__
#define __LLGL_TEXTURE2D_DESC_H__


#include <details/TextureFormat.h>


namespace LLGL
{


struct Texture2DDesc
{
    //! Texture width (in pixels).
    unsigned int    width;

    //! Texture height (in pixels).
    unsigned int    height;

    //! Texture hardware format.
    TextureFormat   format;

    //! Number of MIP-maps. If 0, the number of MIP-maps will be automatically computed.
    unsigned int    mips;
};


} // /namespace LLGL


#endif



// ================================================================================
