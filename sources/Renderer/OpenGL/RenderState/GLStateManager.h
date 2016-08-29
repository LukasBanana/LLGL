/*
 * GLStateManager.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_STATE_MANAGER_H__
#define __LLGL_GL_STATE_MANAGER_H__


#include "GLState.h"
#include "../Buffer/GLVertexBuffer.h"
#include "../Buffer/GLIndexBuffer.h"
#include "../Buffer/GLConstantBuffer.h"
#include "../Texture/GLTexture.h"
#include <LLGL/RenderContextFlags.h>
#include <array>
#include <vector>
#include <stack>


namespace LLGL
{


class GLRenderContext;

class GLStateManager
{

    public:

        /* ----- Common ----- */

        GLStateManager();

        static GLStateManager* active;

        void NotifyRenderTargetHeight(GLint height);

        void SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state);

        /* ----- Boolean states ----- */

        //! Resets all internal states by querying the values from OpenGL.
        void Reset();

        void Set(GLState state, bool value);
        void Enable(GLState state);
        void Disable(GLState state);

        bool IsEnabled(GLState state) const;

        void PushState(GLState state);
        void PopState();
        void PopStates(std::size_t count);

        /* ----- Common states ----- */

        void SetViewports(std::vector<GLViewport>& viewports);
        void SetDepthRanges(std::vector<GLDepthRange>& depthRanges);
        void SetScissors(std::vector<GLScissor>& scissors);

        void SetBlendStates(const std::vector<GLBlend>& blendStates, bool blendEnabled);

        void SetClipControl(GLenum origin, GLenum depth);
        void SetDepthFunc(GLenum func);
        void SetStencilState(GLenum face, const GLStencil& state);
        void SetPolygonMode(GLenum mode);
        void SetCullFace(GLenum face);
        void SetFrontFace(GLenum mode);
        void SetDepthMask(GLboolean flag);

        /* ----- Buffer binding ----- */

        void BindBuffer(GLBufferTarget target, GLuint buffer);
        void BindBufferBase(GLBufferTarget target, GLuint index, GLuint buffer);
        void BindVertexArray(GLuint buffer);

        void ForcedBindBuffer(GLBufferTarget target, GLuint buffer);

        void PushBoundBuffer(GLBufferTarget target);
        void PopBoundBuffer();

        void BindBuffer(const GLVertexBuffer& vertexBuffer);
        void BindBuffer(const GLIndexBuffer& indexBuffer);
        void BindBuffer(const GLConstantBuffer& constantBuffer);
        
        /* ----- Framebuffer binding ----- */

        void BindFrameBuffer(GLFrameBufferTarget target, GLuint framebuffer);

        void PushBoundFrameBuffer(GLFrameBufferTarget target);
        void PopBoundFrameBuffer();

        /* ----- Renderbuffer binding ----- */

        void BindRenderBuffer(GLuint renderbuffer);

        void PushBoundRenderBuffer();
        void PopBoundRenderBuffer();

        /* ----- Texture binding ----- */

        static GLTextureTarget GetTextureTarget(const TextureType type);

        void ActiveTexture(unsigned int layer);

        void BindTexture(GLTextureTarget target, GLuint texture);
        void ForcedBindTexture(GLTextureTarget target, GLuint texture);
        
        void PushBoundTexture(unsigned int layer, GLTextureTarget target);
        void PushBoundTexture(GLTextureTarget target);
        void PopBoundTexture();

        void BindTexture(const GLTexture& texture);
        void ForcedBindTexture(const GLTexture& texture);

        /* ----- Sampler binding ----- */

        void BindSampler(unsigned int layer, GLuint sampler);

        /* ----- Shader binding ----- */

        void BindShaderProgram(GLuint program);

        void PushShaderProgram();
        void PopShaderProgram();

    private:

        /* ----- Functions ----- */

        void SetStencilState(GLenum face, GLStencil& to, const GLStencil& from);
        void SetBlendState(GLuint drawBuffer, const GLBlend& state, bool blendEnabled);
        void AdjustViewport(GLViewport& viewport);
        void AdjustScissor(GLScissor& scissor);

        /* ----- Constants ----- */

        static const std::size_t numTextureLayers       = 32;
        static const std::size_t numStates              = (static_cast<std::size_t>(GLState::PROGRAM_POINT_SIZE) + 1);
        static const std::size_t numBufferTargets       = (static_cast<std::size_t>(GLBufferTarget::UNIFORM_BUFFER) + 1);
        static const std::size_t numFrameBufferTargets  = (static_cast<std::size_t>(GLFrameBufferTarget::READ_FRAMEBUFFER) + 1);
        static const std::size_t numTextureTargets      = (static_cast<std::size_t>(GLTextureTarget::TEXTURE_2D_MULTISAMPLE_ARRAY) + 1);

        /* ----- Structure ----- */

        struct GLCommonState
        {
            GLenum                      depthFunc   = GL_LESS;
            GLStencil                   stencil[2];
            GLenum                      polygonMode = GL_FILL;
            GLenum                      cullFace    = GL_BACK;
            GLenum                      frontFace   = GL_CCW;
            GLboolean                   depthMask   = GL_TRUE;
            ColorRGBAT<GLboolean>       colorMask   = { GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE };
            //std::vector<GLViewport>     viewports;
            //std::vector<GLDepthRange>   depthRanges;
            //std::vector<GLScissor>      scissors;
        };

        struct GLRenderState
        {
            struct StackEntry
            {
                GLState state;
                bool    enabled;
            };

            std::array<bool, numStates> values;
            std::stack<StackEntry>      valueStack;
        };

        struct GLBufferState
        {
            struct StackEntry
            {
                GLBufferTarget  target;
                GLuint          buffer;
            };

            std::array<GLuint, numBufferTargets>    boundBuffers;
            std::stack<StackEntry>                  boundBufferStack;
        };

        struct GLFrameBufferState
        {
            struct StackEntry
            {
                GLFrameBufferTarget target;
                GLuint              buffer;
            };

            std::array<GLuint, numFrameBufferTargets>   boundFrameBuffers;
            std::stack<StackEntry>                      boundFrameBufferStack;
        };

        struct GLRenderBufferState
        {
            GLuint              boundRenderBuffer       = 0;
            std::stack<GLuint>  boundRenderBufferStack;
        };

        struct GLTextureLayer
        {
            std::array<GLuint, numTextureTargets>   boundTextures;
        };

        struct GLTextureState
        {
            struct StackEntry
            {
                unsigned int    layer;
                GLTextureTarget target;
                GLuint          texture;
            };

            unsigned int                                    activeTexture = 0;
            std::array<GLTextureLayer, numTextureLayers>    layers;
            std::stack<StackEntry>                          boundTextureStack;
        };

        struct GLShaderState
        {
            GLuint              boundProgram = 0;
            std::stack<GLuint>  boundProgramStack;
        };

        struct GLSamplerState
        {
            std::array<GLuint, numTextureLayers> boundSamplers;
        };

        /* ----- Members ----- */

        GraphicsAPIDependentStateDescriptor gfxDependentState_;

        GLCommonState                       commonState_;
        GLRenderState                       renderState_;
        GLBufferState                       bufferState_;
        GLFrameBufferState                  frameBufferState_;
        GLRenderBufferState                 renderBufferState_;
        GLTextureState                      textureState_;
        GLShaderState                       shaderState_;
        GLSamplerState                      samplerState_;

        GLTextureLayer*                     activeTextureLayer_ = nullptr;

        bool                                emulateClipControl_ = false;
        GLint                               renderTargetHeight_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
