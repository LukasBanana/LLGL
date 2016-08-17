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
#include <vector>


namespace LLGL
{


struct GLViewport
{
    GLfloat x;
    GLfloat y;
    GLfloat width;
    GLfloat height;
};

struct GLDepthRange
{
    GLdouble minDepth;
    GLdouble maxDepth;
};

struct GLScissor
{
    GLint x;
    GLint y;
    GLint width;
    GLint height;
};

struct GLStencilState
{
    GLenum  func;
    GLenum  sfail;
    GLenum  dpfail;
    GLenum  dppass;
    GLint   ref;
    GLuint  mask;
    GLuint  writeMask;
};


class GLStateManager;

class GLGraphicsPipeline : public GraphicsPipeline
{

    public:

        GLGraphicsPipeline(const GraphicsPipelineDescriptor& desc);

        void Bind(GLStateManager& stateMngr);

    private:

        void BindStencilFace(GLenum face, const GLStencilState& state);

        std::vector<GLViewport>     viewports_;
        std::vector<GLDepthRange>   depthRanges_;
        std::vector<GLScissor>      scissors_;

        bool                        depthTestEnabled_   = false;    // glEnable(GL_DEPTH_TEST)
        bool                        depthWriteEnabled_  = false;    // glDepthMask(GL_TRUE)
        bool                        depthRangeEnabled_  = false;    // glEnable(GL_DEPTH_CLAMP)
        GLenum                      depthCompareOp_     = GL_LESS;

        bool                        stencilTestEnabled_ = false;
        GLStencilState              stencilFront_;
        GLStencilState              stencilBack_;


};


} // /namespace LLGL


#endif



// ================================================================================
