/*
 * DbgRenderTarget.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DBG_RENDER_TARGET_H
#define LLGL_DBG_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>
#include "../RenderState/DbgRenderPass.h"
#include <string>
#include <memory>


namespace LLGL
{


class RenderingDebugger;

class DbgRenderTarget final : public RenderTarget
{

    public:

        #include <LLGL/Backend/RenderTarget.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        DbgRenderTarget(RenderTarget& instance, const RenderTargetDescriptor& desc);

    public:

        RenderTarget&                   instance;
        const RenderTargetDescriptor    desc;
        std::string                     label;

    private:

        std::unique_ptr<DbgRenderPass>  renderPass_;

};


} // /namespace LLGL


#endif



// ================================================================================
