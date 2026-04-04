/*
 * WGRenderTarget.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_RENDER_TARGET_H
#define LLGL_WG_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>


namespace LLGL
{


class WGRenderTarget final : public RenderTarget
{

    public:

        #include <LLGL/Backend/RenderTarget.inl>

    public:

        /* WGRenderTarget(const RenderTargetDescriptor& desc); */

    private:

        // private fields ...

};


} // /namespace LLGL


#endif



// ================================================================================
