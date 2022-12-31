/*
 * NullRenderPass.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        void SetName(const char* name) override;

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
