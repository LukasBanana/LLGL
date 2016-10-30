/*
 * GLStateManager.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_STATE_MANAGER_H
#define LLGL_GL_STATE_MANAGER_H


#include "GLState.h"
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

        void DetermineExtensions();

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
        void BindSamplers(unsigned int first, unsigned int count, const GLuint* samplers);

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

        static const unsigned int numTextureLayers      = 32;
        static const unsigned int numStates             = (static_cast<unsigned int>(GLState::PROGRAM_POINT_SIZE) + 1);
        static const unsigned int numBufferTargets      = (static_cast<unsigned int>(GLBufferTarget::UNIFORM_BUFFER) + 1);
        static const unsigned int numFrameBufferTargets = (static_cast<unsigned int>(GLFrameBufferTarget::READ_FRAMEBUFFER) + 1);
        static const unsigned int numTextureTargets     = (static_cast<unsigned int>(GLTextureTarget::TEXTURE_2D_MULTISAMPLE_ARRAY) + 1);

        #ifdef LLGL_GL_ENABLE_VENDOR_EXT
        static const unsigned int numStatesExt          = (static_cast<unsigned int>(GLStateExt::CONSERVATIVE_RASTERIZATION) + 1);
        #endif

        /* ----- Structure ----- */

        struct GLCommonState
        {
            GLenum      depthFunc       = GL_LESS;
            GLStencil   stencil[2];
            GLenum      cullFace        = GL_BACK;
            GLenum      frontFace       = GL_CCW;
            GLenum      frontFaceAct    = GL_CCW; // actual front face input (without possible inversion)
            GLboolean   depthMask       = GL_TRUE;
            GLint       patchVertices_  = 0;
            ColorRGBAf  blendColor      = { 0.0f, 0.0f, 0.0f, 0.0f };
            #ifndef LLGL_GL_OPENGLES
            GLenum      polygonMode     = GL_FILL;
            GLenum      logicOpCode     = GL_COPY;
            #endif
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

};


} // /namespace LLGL


#endif



// ================================================================================
