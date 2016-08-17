/*
 * GLGraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_GRAPHICS_PIPELINE_H__
#define __LLGL_GL_GRAPHICS_PIPELINE_H__


#include "../OpenGL.h"
#include "GLStateManager.h"
#include <LLGL/GraphicsPipeline.h>
#include <vector>


namespace LLGL
{


class GLGraphicsPipeline : public GraphicsPipeline
{

    public:

        GLGraphicsPipeline(const GraphicsPipelineDescriptor& desc);

        void Bind(GLStateManager& stateMngr);

    private:

        std::vector<GLViewport>     viewports_;
        std::vector<GLDepthRange>   depthRanges_;
        std::vector<GLScissor>      scissors_;

        bool                        depthTestEnabled_   = false;    // glEnable(GL_DEPTH_TEST)
        bool                        depthWriteEnabled_  = false;    // glDepthMask(GL_TRUE)
        bool                        depthRangeEnabled_  = false;    // glEnable(GL_DEPTH_CLAMP)
        GLenum                      depthFunc_          = GL_LESS;

        bool                        stencilTestEnabled_ = false;
        GLStencil                   stencilFront_;
        GLStencil                   stencilBack_;


};


} // /namespace LLGL


#endif



// ================================================================================
