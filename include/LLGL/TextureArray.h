/*
 * TextureArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TEXTURE_ARRAY_H
#define LLGL_TEXTURE_ARRAY_H


#include "Export.h"


namespace LLGL
{


//! Array of textures interface.
class LLGL_EXPORT TextureArray
{

    public:

        TextureArray(const TextureArray&) = delete;
        TextureArray& operator = (const TextureArray&) = delete;

        virtual ~TextureArray()
        {
        }

    protected:

        TextureArray() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
