/*
 * GLRenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_RENDER_CONTEXT_H__
#define __LLGL_GL_RENDER_CONTEXT_H__


#include <LLGL/Window.h>
#include <LLGL/RenderContext.h>
#include "OpenGL.h"

#if defined(_WIN32)
#   include "Win32/Win32GLPlatformContext.h"
#endif


namespace LLGL
{


class GLRenderContext : public RenderContext
{

    public:

        GLRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window, GLRenderContext* sharedRenderContext);

        ~GLRenderContext();

        std::string GetVersion() const override;

        void Present() override;

    private:

        void CreateContext(GLRenderContext* sharedRenderContext);
        void DeleteContext();

        RenderContextDescriptor desc_;
        std::shared_ptr<Window> window_;
        GLPlatformContext       context_;

};


} // /namespace LLGL


#endif



// ================================================================================
