/*
 * GLGraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_GRAPHICS_PIPELINE_H__
#define __LLGL_GL_GRAPHICS_PIPELINE_H__


#include "../OpenGL.h"
#include <LLGL/GraphicsPipeline.h>


namespace LLGL
{


class GLStateManager;

class GLGraphicsPipeline : public GraphicsPipeline
{

    public:

        GLGraphicsPipeline(const GraphicsPipelineDescriptor& desc);

        void Bind(GLStateManager& stateMngr);

    private:

        std::vector<Viewport>   viewports_;
        std::vector<Scissor>    scissors_;

        bool                    depthTestEnabled_   = false;    // glEnable(GL_DEPTH_TEST)
        bool                    depthWriteEnabled_  = false;    // glDepthMask(GL_TRUE)
        bool                    depthRangeEnabled_  = false;    // glEnable(GL_DEPTH_CLAMP)

};


} // /namespace LLGL


#endif



// ================================================================================
