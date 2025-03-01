/*
 * GLRenderSystem.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_RENDER_SYSTEM_H
#define LLGL_GL_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include "Ext/GLExtensionRegistry.h"
#include "../ContainerTypes.h"

#include "Command/GLCommandQueue.h"
#include "Command/GLCommandBuffer.h"
#include "GLSwapChain.h"
#include "Platform/GLContextManager.h"

#include "Buffer/GLBuffer.h"
#include "Buffer/GLBufferArray.h"

#include "Shader/GLShader.h"
#include "Shader/GLShaderProgram.h"

#include "Texture/GLTexture.h"
#include "Texture/GLSampler.h"
#include "Texture/GLRenderTarget.h"
#include "Texture/GLEmulatedSampler.h"

#include "RenderState/GLQueryHeap.h"
#include "RenderState/GLFence.h"
#include "RenderState/GLRenderPass.h"
#include "RenderState/GLPipelineLayout.h"
#include "RenderState/GLPipelineCache.h"
#include "RenderState/GLPipelineState.h"
#include "RenderState/GLResourceHeap.h"

#include "../ProxyPipelineCache.h"

#include <string>
#include <memory>
#include <vector>
#include <set>


namespace LLGL
{


class GLContext;
struct GLPixelFormat;

class GLRenderSystem final : public RenderSystem
{

    public:

        #include <LLGL/Backend/RenderSystem.inl>

    public:

        GLRenderSystem(const RenderSystemDescriptor& renderSystemDesc);
        ~GLRenderSystem();

    public:

        inline bool IsBreakOnErrorEnabled() const
        {
            return isBreakOnErrorEnabled_;
        }

    private:

        #include <LLGL/Backend/RenderSystem.Internal.inl>

    private:

        // Creates a GL context once or creates a new one if there is no compatible one with the specified pixel format.
        void CreateGLContextOnce();

        // Updates the renderer capabilities information and enables the debug layer for the new GLContext if enabled.
        void RegisterNewGLContext(GLContext& context, const GLPixelFormat& pixelFormat);

        void EnableDebugCallback(bool enable = true);

        GLBuffer* CreateGLBuffer(const BufferDescriptor& desc, const void* initialData);

        void ValidateGLTextureType(const TextureType type);

    private:

        /* ----- Hardware object containers ----- */

        GLContextManager                        contextMngr_;
        GLCommandQueue                          commandQueue_;
        bool                                    debugContext_           = false;
        bool                                    isBreakOnErrorEnabled_  = false;

        HWObjectContainer<GLSwapChain>          swapChains_;
        HWObjectContainer<GLCommandBuffer>      commandBuffers_;
        HWObjectContainer<GLBuffer>             buffers_;
        HWObjectContainer<GLBufferArray>        bufferArrays_;
        HWObjectContainer<GLTexture>            textures_;
        HWObjectContainer<GLSampler>            samplers_;
        HWObjectContainer<GLEmulatedSampler>    emulatedSamplers_;
        HWObjectContainer<GLRenderPass>         renderPasses_;
        HWObjectContainer<GLRenderTarget>       renderTargets_;
        HWObjectContainer<GLShader>             shaders_;
        HWObjectContainer<GLPipelineLayout>     pipelineLayouts_;
        HWObjectInstance<ProxyPipelineCache>    pipelineCacheProxy_;
        HWObjectContainer<GLPipelineCache>      pipelineCaches_;
        HWObjectContainer<GLPipelineState>      pipelineStates_;
        HWObjectContainer<GLResourceHeap>       resourceHeaps_;
        HWObjectContainer<GLQueryHeap>          queryHeaps_;
        HWObjectContainer<GLFence>              fences_;

};


} // /namespace LLGL


#endif



// ================================================================================
