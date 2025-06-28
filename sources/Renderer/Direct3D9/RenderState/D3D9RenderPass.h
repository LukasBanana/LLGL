/*
 * D3D9RenderPass.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_RENDER_PASS_H
#define LLGL_D3D9_RENDER_PASS_H


#include <LLGL/RenderPass.h>
#include <LLGL/RenderPassFlags.h>
#include <string>


namespace LLGL
{


class D3D9RenderPass final : public RenderPass
{

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D9RenderPass(const RenderPassDescriptor& desc);

    public:

        const RenderPassDescriptor desc;

    private:

        std::string label_;

};


} // /namespace LLGL


#endif



// ================================================================================
