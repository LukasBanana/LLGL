/*
 * GLStateManager.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_STATE_MANAGER_H__
#define __LLGL_GL_STATE_MANAGER_H__


#include "GLState.h"
#include "../Ext/GLExtensionViewer.h"
#include "../Buffer/GLBuffer.h"
#include "../Texture/GLTexture.h"
#include <LLGL/RenderContextFlags.h>
#include <array>
#include <vector>
#include <stack>


namespace LLGL
{


class GLRenderSystem;

class GLStateManager
{

    public:

        /* ----- Common ----- */

        GLStateManager();

        static GLStateManager* active;

        void DetermineExtensions(GLExtensionViewer& extensionViewer);

        //! Notifies the state manager about a new render-target height.
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

        #ifdef LLGL_GL_ENABLE_VENDOR_EXT

        void Set(GLStateExt state, bool value);
        void Enable(GLStateExt state);
        void Disable(GLStateExt state);

        bool IsEnabled(GLStateExt state) const;

        #endif

        /* ----- Common states ----- */

        void SetViewport(GLViewport& viewport);
        void SetViewportArray(std::vector<GLViewport>&& viewports);

        void SetDepthRange(GLDepthRange& depthRange);
        void SetDepthRangeArray(std::vector<GLDepthRange>&& depthRanges);

        void SetScissor(GLScissor& scissor);
        void SetScissorArray(std::vector<GLScissor>&& scissors);

        void SetBlendStates(const std::vector<GLBlend>& blendStates, bool blendEnabled);

        void SetClipControl(GLenum origin, GLenum depth);
        void SetDepthFunc(GLenum func);
        void SetStencilState(GLenum face, const GLStencil& state);
        void SetPolygonMode(GLenum mode);
        void SetCullFace(GLenum face);
        void SetFrontFace(GLenum mode);
        void SetDepthMask(GLboolean flag);
        void SetPatchVertices(GLint patchVertices);
        void SetBlendColor(const ColorRGBAf& color);
        void SetLogicOp(GLenum opcode);

        /* ----- Buffer binding ----- */

        void BindBuffer(GLBufferTarget target, GLuint buffer);
        void BindBufferBase(GLBufferTarget target, GLuint index, GLuint buffer);
        void BindBuffersBase(GLBufferTarget target, GLuint first, GLsizei count, const GLuint* buffers);

        void BindVertexArray(GLuint vertexArray);

        /**
        \brief Binds the specified index buffer as soon as the next VAO with "BindVertexArray" is bound.
        \see BindVertexArray
        */
        void DeferredBindIndexBuffer(GLuint buffer);

        void PushBoundBuffer(GLBufferTarget target);
        void PopBoundBuffer();

        void BindBuffer(const GLBuffer& buffer);

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
        void BindTextures(GLuint first, GLsizei count, const GLTextureTarget* targets, const GLuint* textures);
        
        void PushBoundTexture(unsigned int layer, GLTextureTarget target);
        void PushBoundTexture(GLTextureTarget target);
        void PopBoundTexture();

        void BindTexture(const GLTexture& texture);

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

        void AssertExtViewportArray();

        void SetActiveTextureLayer(unsigned int layer);

        /* ----- Constants ----- */

        static const std::size_t numTextureLayers       = 32;
        static const std::size_t numStates              = (static_cast<std::size_t>(GLState::PROGRAM_POINT_SIZE) + 1);
        static const std::size_t numBufferTargets       = (static_cast<std::size_t>(GLBufferTarget::UNIFORM_BUFFER) + 1);
        static const std::size_t numFrameBufferTargets  = (static_cast<std::size_t>(GLFrameBufferTarget::READ_FRAMEBUFFER) + 1);
        static const std::size_t numTextureTargets      = (static_cast<std::size_t>(GLTextureTarget::TEXTURE_2D_MULTISAMPLE_ARRAY) + 1);

        #ifdef LLGL_GL_ENABLE_VENDOR_EXT
        static const std::size_t numStatesExt           = (static_cast<std::size_t>(GLStateExt::CONSERVATIVE_RASTERIZATION) + 1);
        #endif

        /* ----- Structure ----- */

        struct GLCommonState
        {
            GLenum                      depthFunc       = GL_LESS;
            GLStencil                   stencil[2];
            GLenum                      polygonMode     = GL_FILL;
            GLenum                      cullFace        = GL_BACK;
            GLenum                      frontFace       = GL_CCW;
            GLenum                      frontFaceAct    = GL_CCW; // actual front face input (without possible inversion)
            GLboolean                   depthMask       = GL_TRUE;
            GLint                       patchVertices_  = 0;
            ColorRGBAT<GLboolean>       colorMask       = { GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE };
            ColorRGBAf                  blendColor      = { 0.0f, 0.0f, 0.0f, 0.0f };
            GLenum                      logicOpCode     = GL_COPY;
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

        #ifdef LLGL_GL_ENABLE_VENDOR_EXT

        struct GLRenderStateExt
        {
            struct ValueEntry
            {
                GLenum  cap     = 0;
                bool    enabled = false;
            };

            std::array<ValueEntry, numStatesExt> values;
        };

        #endif

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

        struct GLVertexArrayState
        {
            GLuint boundVertexArray         = 0;
            GLuint deferredBoundIndexBuffer = 0;
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

        struct GLExtSupport
        {
            bool viewportArray      = false; // GL_ARB_viewport_array
            bool clipControl        = false; // GL_ARB_clip_control
            bool drawBuffersBlend   = false; // GL_ARB_draw_buffers_blend
            bool multiBind          = false; // GL_ARB_multi_bind
        };

        /* ----- Members ----- */

        GraphicsAPIDependentStateDescriptor gfxDependentState_;

        GLCommonState                       commonState_;
        GLRenderState                       renderState_;
        GLBufferState                       bufferState_;
        GLFrameBufferState                  frameBufferState_;
        GLRenderBufferState                 renderBufferState_;
        GLTextureState                      textureState_;
        GLVertexArrayState                  vertexArrayState_;
        GLShaderState                       shaderState_;
        GLSamplerState                      samplerState_;

        #ifdef LLGL_GL_ENABLE_VENDOR_EXT
        GLRenderStateExt                    renderStateExt_;
        #endif

        GLTextureLayer*                     activeTextureLayer_ = nullptr;

        bool                                emulateClipControl_ = false;
        GLint                               renderTargetHeight_ = 0;

        GLExtSupport                        extension_;

};


} // /namespace LLGL


#endif



// ================================================================================
