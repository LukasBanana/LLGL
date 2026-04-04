/*
 * WGTexture.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_TEXTURE_H
#define LLGL_WG_TEXTURE_H


#include <LLGL/Texture.h>


namespace LLGL
{


class WGTexture final : public Texture
{

    public:

        #include <LLGL/Backend/Texture.inl>

    public:

        /* WGTexture(const TextureDescriptor& desc); */

    private:

        // private fields ...

};


} // /namespace LLGL


#endif



// ================================================================================
