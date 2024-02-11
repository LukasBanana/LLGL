/*
 * NullRenderPass.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NULL_RENDER_PASS_H
#define LLGL_NULL_RENDER_PASS_H


#include <LLGL/RenderPass.h>
#include <LLGL/RenderPassFlags.h>
#include <string>


namespace LLGL
{


class NullRenderPass final : public RenderPass
{

    public:

        void SetDebugName(const char* name) override;

    public:

        NullRenderPass(const RenderPassDescriptor& desc);

    public:

        const RenderPassDescriptor desc;

    private:

        std::string label_;

};


} // /namespace LLGL


#endif



// ================================================================================
