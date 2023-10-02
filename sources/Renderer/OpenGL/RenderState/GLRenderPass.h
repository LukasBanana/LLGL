/*
 * GLRenderPass.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_RENDER_PASS_H
#define LLGL_GL_RENDER_PASS_H


#include <LLGL/RenderPass.h>
#include <LLGL/RenderPassFlags.h>
#include <cstdint>
#include "../OpenGL.h"


namespace LLGL
{


class GLRenderPass final : public RenderPass
{

    public:

        GLRenderPass(const RenderPassDescriptor& desc);

        // Returns the number of color attachments used for this render pass.
        inline std::uint8_t GetNumColorAttachments() const
        {
            return numColorAttachments_;
        }

        // Specifies which buffer groups are meant to be cleared when a render pass begins.
        inline GLbitfield GetClearMask() const
        {
            return clearMask_;
        }

        // Returns the array of color attachment indices that are meant to be cleared when a render pass begins (value of 0xFF ends the list).
        inline const std::uint8_t* GetClearColorAttachments() const
        {
            return clearColorAttachments_;
        }

    private:

        GLbitfield      clearMask_                                              = 0;
        std::uint8_t    clearColorAttachments_[LLGL_MAX_NUM_COLOR_ATTACHMENTS]  = {};
        std::uint8_t    numColorAttachments_                                    = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
