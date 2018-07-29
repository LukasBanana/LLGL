/*
 * GLStateManager.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_STATE_MANAGER_H
#define LLGL_GL_STATE_MANAGER_H


#include "GLState.h"
#include "../Buffer/GLBuffer.h"
#include "../Texture/GLTexture.h"
#include <LLGL/CommandBufferFlags.h>
#include <array>
#include <stack>
#include <cstdint>


namespace LLGL
{


class GLRenderTarget;

/*
TODO:
Refactore this entire class and maybe rename it to "GLContextState".
Don't store every single state, instead separate the states into groups of states like "GLRasterizerState", "GLBlendState" etc.
*/
// OpenGL state machine manager that tries to reduce GL state changes.
class GLStateManager
{

    public:

        /* ----- Common ----- */

        GLStateManager();
        ~GLStateManager();

        // Active state manager. Each GL context has its own states, thus its own state manager.
        static GLStateManager* active;

        // Queries all supported and available GL extensions and limitations, then stores it internally (must be called once a GL context has been created).
        void DetermineExtensionsAndLimits();

        //TODO: viewports and scissors must be updated!
        // Notifies the state manager about a new render-target height.
        void NotifyRenderTargetHeight(GLint height);

        // Sets and applies the specified OpenGL specific render state.
        void SetGraphicsAPIDependentState(const OpenGLDependentStateDescriptor& stateDesc);

        /* ----- Boolean states ----- */

        // Resets all internal states by querying the values from OpenGL.
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
        void SetViewportArray(GLuint first, GLsizei count, GLViewport* viewports);

        void SetDepthRange(const GLDepthRange& depthRange);
        void SetDepthRangeArray(GLuint first, GLsizei count, const GLDepthRange* depthRanges);

        void SetScissor(GLScissor& scissor);
        void SetScissorArray(GLuint first, GLsizei count, GLScissor* scissors);

        void SetClipControl(GLenum origin, GLenum depth);
        void SetPolygonMode(GLenum mode);
        void SetPolygonOffset(GLfloat factor, GLfloat units, GLfloat clamp);
        void SetCullFace(GLenum face);
        void SetFrontFace(GLenum mode);
        void SetPatchVertices(GLint patchVertices);
        void SetLineWidth(GLfloat width);
        #if 0//TODO
        void SetSampleMask(GLuint maskNumber, GLbitfield mask);
        #endif

        /* ----- Blend states ----- */

        void SetBlendColor(const GLfloat (&color)[4]);
        void SetLogicOp(GLenum opcode);

        void SetBlendStates(
            std::size_t     numBlendStates,
            const GLBlend*  blendStates,
            bool            anyBlendTargetEnabled
        );

        void PushColorMaskAndEnable();
        void PopColorMask();

        /* ----- Depth-stencil states ----- */

        void SetDepthFunc(GLenum func);
        void SetDepthMask(GLboolean flag);
        void SetStencilState(GLenum face, const GLStencil& state);

        void PushDepthMaskAndEnable();
        void PopDepthMask();

        /* ----- Buffer ----- */

        static GLBufferTarget GetBufferTarget(const BufferType type);

        void BindBuffer(GLBufferTarget target, GLuint buffer);
        void BindBufferBase(GLBufferTarget target, GLuint index, GLuint buffer);
        void BindBuffersBase(GLBufferTarget target, GLuint first, GLsizei count, const GLuint* buffers);

        void BindVertexArray(GLuint vertexArray);

        void NotifyVertexArrayRelease(GLuint vertexArray);

        /**
        \brief Binds the specified GL_ELEMENT_ARRAY_BUFFER (i.e. index buffer) to the next VAO (or the current one).
        \see BindVertexArray
        */
        void BindElementArrayBufferToVAO(GLuint buffer);

        void PushBoundBuffer(GLBufferTarget target);
        void PopBoundBuffer();

        void BindBuffer(const GLBuffer& buffer);

        void NotifyBufferRelease(GLuint buffer, GLBufferTarget target);

        /* ----- Framebuffer ----- */

        void BindRenderTarget(GLRenderTarget* renderTarget);
        void BindFramebuffer(GLFramebufferTarget target, GLuint framebuffer);

        void PushBoundFramebuffer(GLFramebufferTarget target);
        void PopBoundFramebuffer();

        void NotifyFramebufferRelease(GLuint framebuffer);

        GLRenderTarget* GetBoundRenderTarget() const;

        /* ----- Renderbuffer ----- */

        void BindRenderbuffer(GLuint renderbuffer);

        void NotifyRenderbufferRelease(GLuint renderbuffer);

        /* ----- Texture ----- */

        static GLTextureTarget GetTextureTarget(const TextureType type);

        void ActiveTexture(std::uint32_t layer);

        void BindTexture(GLTextureTarget target, GLuint texture);
        void BindTextures(GLuint first, GLsizei count, const GLTextureTarget* targets, const GLuint* textures);

        void PushBoundTexture(std::uint32_t layer, GLTextureTarget target);
        void PushBoundTexture(GLTextureTarget target);
        void PopBoundTexture();

        void BindTexture(const GLTexture& texture);

        void NotifyTextureRelease(GLuint texture, GLTextureTarget target);

        /* ----- Sampler ----- */

        void BindSampler(GLuint layer, GLuint sampler);
        void BindSamplers(GLuint first, GLsizei count, const GLuint* samplers);

        void NotifySamplerRelease(GLuint sampler);

        /* ----- Shader Program ----- */

        void BindShaderProgram(GLuint program);

        void PushShaderProgram();
        void PopShaderProgram();

        void NotifyShaderProgramRelease(GLuint program);

    private:

        /* ----- Functions ----- */

        void SetDrawBufferBlendState(GLuint drawBufferIndex, const GLBlend& state, bool blendEnabled);
        void SetAllDrawBufferBlendState(const GLBlend& state, bool blendEnabled);
        void SetAllDrawBufferBlendStateDefault(bool blendEnabled);

        void AdjustViewport(GLViewport& viewport);
        void AdjustScissor(GLScissor& scissor);

        void AssertViewportLimit(GLuint first, GLsizei count);
        void AssertExtViewportArray();

        void SetActiveTextureLayer(std::uint32_t layer);

        void DetermineLimits();

        #ifdef LLGL_GL_ENABLE_VENDOR_EXT
        void DetermineVendorSpecificExtensions();
        #endif

        /* ----- Constants ----- */

        static const std::uint32_t numTextureLayers         = 32;
        static const std::uint32_t numStates                = (static_cast<std::uint32_t>(GLState::PROGRAM_POINT_SIZE) + 1);
        static const std::uint32_t numBufferTargets         = (static_cast<std::uint32_t>(GLBufferTarget::UNIFORM_BUFFER) + 1);
        static const std::uint32_t numFramebufferTargets    = (static_cast<std::uint32_t>(GLFramebufferTarget::READ_FRAMEBUFFER) + 1);
        static const std::uint32_t numTextureTargets        = (static_cast<std::uint32_t>(GLTextureTarget::TEXTURE_2D_MULTISAMPLE_ARRAY) + 1);

        #ifdef LLGL_GL_ENABLE_VENDOR_EXT
        static const std::uint32_t numStatesExt             = (static_cast<std::uint32_t>(GLStateExt::CONSERVATIVE_RASTERIZATION) + 1);
        #endif

        /* ----- Structures ----- */

        // GL limitations required for validation of state parameters
        struct GLLimits
        {
            GLint       maxViewports        = 16;               // must be at least 16
            GLfloat     lineWidthRange[2]   = { 1.0f, 1.0f };   // minimal range of both <aliased> and <smooth> line width range
        };

        //TODO: separate this in to "GLRasterizerState"
        // Common GL states
        struct GLCommonState
        {
            GLenum      polygonMode     = GL_FILL;
            GLfloat     offsetFactor    = 0.0f;
            GLfloat     offsetUnits     = 0.0f;
            GLfloat     offsetClamp     = 0.0f;
            GLenum      cullFace        = GL_BACK;
            GLenum      frontFace       = GL_CCW;
            GLenum      frontFaceAct    = GL_CCW; // actual front face input (without possible inversion)
            GLint       patchVertices_  = 0;
            GLfloat     lineWidth       = 1.0f;
        };

        struct GLBlendState
        {
            GLfloat     blendColor[4]       = { 0.0f, 0.0f, 0.0f, 0.0f };
            GLenum      logicOpCode         = GL_COPY;
            GLboolean   colorMasks[32][4]   = {};
            GLuint      numDrawBuffers      = 0;
            bool        colorMaskOnStack    = false;
        };

        struct GLDepthStencilState
        {
            GLenum      depthFunc       = GL_LESS;
            GLboolean   depthMask       = GL_TRUE;
            GLStencil   stencil[2];
            GLboolean   depthMaskStack  = GL_TRUE;
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

        struct GLFramebufferState
        {
            struct StackEntry
            {
                GLFramebufferTarget target;
                GLuint              framebuffer;
            };

            std::array<GLuint, numFramebufferTargets>   boundFramebuffers;
            std::stack<StackEntry>                      boundFramebufferStack;
            GLRenderTarget*                             boundRenderTarget       = nullptr;
        };

        struct GLRenderbufferState
        {
            GLuint boundRenderbuffer = 0;
        };

        struct GLTextureLayer
        {
            std::array<GLuint, numTextureTargets> boundTextures;
        };

        struct GLTextureState
        {
            struct StackEntry
            {
                std::uint32_t   layer;
                GLTextureTarget target;
                GLuint          texture;
            };

            std::uint32_t                                   activeTexture = 0;
            std::array<GLTextureLayer, numTextureLayers>    layers;
            std::stack<StackEntry>                          boundTextureStack;
        };

        struct GLVertexArrayState
        {
            GLuint boundVertexArray         = 0;
            GLuint boundElementArrayBuffer  = 0;
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

        GLLimits                        limits_;

        OpenGLDependentStateDescriptor  apiDependentState_;

        GLCommonState                   commonState_;
        GLDepthStencilState             depthStencilState_;
        GLBlendState                    blendState_;
        GLRenderState                   renderState_;
        GLBufferState                   bufferState_;
        GLFramebufferState              framebufferState_;
        GLRenderbufferState             renderbufferState_;
        GLTextureState                  textureState_;
        GLVertexArrayState              vertexArrayState_;
        GLShaderState                   shaderState_;
        GLSamplerState                  samplerState_;

        #ifdef LLGL_GL_ENABLE_VENDOR_EXT
        GLRenderStateExt                renderStateExt_;
        #endif

        GLTextureLayer*                 activeTextureLayer_ = nullptr;

        bool                            emulateClipControl_ = false;
        GLint                           renderTargetHeight_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
