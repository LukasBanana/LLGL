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
#include "RenderState/GLStateManager.h"
#include "Platform/GLContext.h"
#include <memory>

#ifndef __APPLE__
#include <LLGL/Platform/NativeHandle.h>
#endif


namespace LLGL
{


class GLRenderTarget;

class GLRenderContext : public RenderContext
{

    public:

        /* ----- Common ----- */

        GLRenderContext(RenderContextDescriptor desc, const std::shared_ptr<Window>& window, GLRenderContext* sharedRenderContext);

        void Present() override;

        /* ----- Configuration ----- */

        void SetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        void SetVsync(const VsyncDescriptor& vsyncDesc) override;

        /* ----- GLRenderContext specific functions ----- */

        static bool GLMakeCurrent(GLRenderContext* renderContext);

        inline const std::shared_ptr<GLStateManager>& GetStateManager() const
        {
            return stateMngr_;
        }

    private:

        struct RenderState
        {
            GLenum      drawMode            = GL_TRIANGLES;
            GLenum      indexBufferDataType = GL_UNSIGNED_INT;
            GLintptr    indexBufferStride   = 4;
        };

        void InitRenderStates();
        void UpdateSwapInterval();

        #ifndef __APPLE__
        void GetNativeContextHandle(NativeContextHandle& windowContext);
        #endif

        RenderContextDescriptor         desc_;

        std::unique_ptr<GLContext>      context_;

        std::shared_ptr<GLStateManager> stateMngr_;
        RenderState                     renderState_;

        GLRenderTarget*                 boundRenderTarget_  = nullptr;

        GLint                           contextHeight_      = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
