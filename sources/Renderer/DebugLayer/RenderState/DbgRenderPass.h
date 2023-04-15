/*
 * DbgRenderPass.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DBG_RENDER_PASS_H
#define LLGL_DBG_RENDER_PASS_H


#include <LLGL/RenderPass.h>
#include <LLGL/RenderPassFlags.h>
#include <string>


namespace LLGL
{


class DbgRenderPass final : public RenderPass
{

    public:

        void SetName(const char* name) override;

    public:

        DbgRenderPass(RenderPass& instance, const RenderPassDescriptor& desc);
        DbgRenderPass(const RenderPass& instance, const RenderPassDescriptor& desc);

        std::uint32_t NumEnabledColorAttachments() const;

    public:

        const RenderPass&           instance;
        RenderPass* const           mutableInstance;
        const RenderPassDescriptor  desc;
        std::string                 label;

};


} // /namespace LLGL


#endif



// ================================================================================
