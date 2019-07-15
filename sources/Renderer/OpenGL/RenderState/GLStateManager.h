/*
 * GLStateManager.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_STATE_MANAGER_H
#define LLGL_GL_STATE_MANAGER_H


#include "GLState.h"
#include <LLGL/TextureFlags.h>
#include <LLGL/CommandBufferFlags.h>
#include <array>
#include <stack>
#include <cstdint>


namespace LLGL
{


class RenderTarget;
class RenderPass;
class GLRenderTarget;
class GLRenderContext;
class GLBuffer;
class GLTexture;
class GLDepthStencilState;
class GLRasterizerState;
class GLBlendState;
class GLRenderPass;

// OpenGL state machine manager that keeps track of certain GL states.
class GLStateManager
{

    public:

        // Maximal number of resource slots.
        static const std::uint32_t g_maxNumResourceSlots = 64;

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
        //void SetSampleMask(GLuint maskNumber, GLbitfield mask);
        #endif

        /* ----- Depth-stencil states ----- */

        void NotifyDepthStencilStateRelease(GLDepthStencilState* depthStencilState);

        void BindDepthStencilState(GLDepthStencilState* depthStencilState);

        void SetDepthFunc(GLenum func);
        void SetDepthMask(GLboolean flag);

        /* ----- Rasterizer states ----- */

        void NotifyRasterizerStateRelease(GLRasterizerState* rasterizerState);

        void BindRasterizerState(GLRasterizerState* rasterizerState);

        /* ----- Blend states ----- */

        void NotifyBlendStateRelease(GLBlendState* blendState);

        void BindBlendState(GLBlendState* blendState);

        void SetBlendColor(const GLfloat (&color)[4]);
        void SetLogicOp(GLenum opcode);

        /* ----- Buffer ----- */

        static GLenum ToGLBufferTarget(GLBufferTarget target);

        void BindBuffer(GLBufferTarget target, GLuint buffer);
        void BindBufferBase(GLBufferTarget target, GLuint index, GLuint buffer);
        void BindBuffersBase(GLBufferTarget target, GLuint first, GLsizei count, const GLuint* buffers);
        void UnbindBuffersBase(GLBufferTarget target, GLuint first, GLsizei count);

        void BindVertexArray(GLuint vertexArray);

        void BindGLBuffer(const GLBuffer& buffer);

        void NotifyVertexArrayRelease(GLuint vertexArray);

        /**
        \brief Binds the specified GL_ELEMENT_ARRAY_BUFFER (i.e. index buffer) to the next VAO (or the current one).
        \see BindVertexArray
        */
        void BindElementArrayBufferToVAO(GLuint buffer);

        void PushBoundBuffer(GLBufferTarget target);
        void PopBoundBuffer();

        void NotifyBufferRelease(GLuint buffer, GLBufferTarget target);
        void NotifyBufferRelease(const GLBuffer& buffer);

        // Disables all previous enabled vertex attrib arrays, and sets the specified index as the new highest enabled index.
        void DisableVertexAttribArrays(GLuint firstIndex);

        /* ----- Framebuffer ----- */

        void BindGLRenderTarget(GLRenderTarget* renderTarget);
        void BindFramebuffer(GLFramebufferTarget target, GLuint framebuffer);

        void PushBoundFramebuffer(GLFramebufferTarget target);
        void PopBoundFramebuffer();

        void NotifyFramebufferRelease(GLuint framebuffer);
        void NotifyGLRenderTargetRelease(GLRenderTarget* renderTarget);

        GLRenderTarget* GetBoundRenderTarget() const;

        /* ----- Renderbuffer ----- */

        void BindRenderbuffer(GLuint renderbuffer);

        void NotifyRenderbufferRelease(GLuint renderbuffer);

        /* ----- Texture ----- */

        static GLTextureTarget GetTextureTarget(const TextureType type);

        void ActiveTexture(std::uint32_t layer);

        void BindTexture(GLTextureTarget target, GLuint texture);
        void BindTextures(GLuint first, GLsizei count, const GLTextureTarget* targets, const GLuint* textures);
        void UnbindTextures(GLuint first, GLsizei count);

        void PushBoundTexture(std::uint32_t layer, GLTextureTarget target);
        void PushBoundTexture(GLTextureTarget target);
        void PopBoundTexture();

        void BindGLTexture(const GLTexture& texture);

        void NotifyTextureRelease(GLuint texture, GLTextureTarget target);

        /* ----- Sampler ----- */

        void BindSampler(GLuint layer, GLuint sampler);
        void BindSamplers(GLuint first, GLsizei count, const GLuint* samplers);
        void UnbindSamplers(GLuint first, GLsizei count);

        void NotifySamplerRelease(GLuint sampler);

        /* ----- Shader Program ----- */

        void BindShaderProgram(GLuint program);

        void NotifyShaderProgramRelease(GLuint program);

        GLuint GetBoundShaderProgram() const;

        /* ----- Render pass ----- */

        void BindRenderPass(
            RenderTarget&       renderTarget,
            const RenderPass*   renderPass,
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues,
            const GLClearValue& defaultClearValue
        );

        void Clear(long flags);
        void ClearBuffers(std::uint32_t numAttachments, const AttachmentClear* attachments);

    private:

        void AdjustViewport(GLViewport& viewport);
        void AdjustScissor(GLScissor& scissor);

        void AssertViewportLimit(GLuint first, GLsizei count);
        void AssertExtViewportArray();

        void SetActiveTextureLayer(std::uint32_t layer);

        void DetermineLimits();

        #ifdef LLGL_GL_ENABLE_VENDOR_EXT
        void DetermineVendorSpecificExtensions();
        #endif

        /* ----- Stacks ----- */

        void PushDepthMaskAndEnable();
        void PopDepthMask();

        void PushColorMaskAndEnable();
        void PopColorMask();

        /* ----- Render pass ----- */

        // Blits the currently bound render target
        void BlitBoundRenderTarget();

        void BindAndBlitRenderTarget(GLRenderTarget& renderTargetGL);
        void BindAndBlitRenderContext(GLRenderContext& renderContextGL);

        void ClearAttachmentsWithRenderPass(
            const GLRenderPass& renderPassGL,
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues,
            const GLClearValue& defaultClearValue
        );

        std::uint32_t ClearColorBuffers(
            const std::uint8_t* colorBuffers,
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues,
            std::uint32_t&      idx,
            const GLClearValue& defaultClearValue
        );

    private:

        static const std::uint32_t numTextureLayers         = 32;
        static const std::uint32_t numStates                = (static_cast<std::uint32_t>(GLState::PROGRAM_POINT_SIZE) + 1);
        static const std::uint32_t numBufferTargets         = (static_cast<std::uint32_t>(GLBufferTarget::UNIFORM_BUFFER) + 1);
        static const std::uint32_t numFramebufferTargets    = (static_cast<std::uint32_t>(GLFramebufferTarget::READ_FRAMEBUFFER) + 1);
        static const std::uint32_t numTextureTargets        = (static_cast<std::uint32_t>(GLTextureTarget::TEXTURE_2D_MULTISAMPLE_ARRAY) + 1);

        #ifdef LLGL_GL_ENABLE_VENDOR_EXT
        static const std::uint32_t numStatesExt             = (static_cast<std::uint32_t>(GLStateExt::CONSERVATIVE_RASTERIZATION) + 1);
        #endif

    private:

        // GL limitations required for validation of state parameters
        struct GLLimits
        {
            GLint       maxViewports        = 16;               // must be at least 16
            GLfloat     lineWidthRange[2]   = { 1.0f, 1.0f };   // minimal range of both <aliased> and <smooth> line width range
        };

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
            GLint       patchVertices   = 0;
            GLfloat     lineWidth       = 1.0f;

            GLenum      depthFunc       = GL_LESS;
            GLboolean   depthMask       = GL_TRUE;
            GLboolean   cachedDepthMask = GL_TRUE;

            GLfloat     blendColor[4]   = { 0.0f, 0.0f, 0.0f, 0.0f };
            GLenum      logicOpCode     = GL_COPY;
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
            GLuint                                  lastVertexAttribArray   = 0;
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

            std::uint32_t                                   activeTexture       = 0;
            std::array<GLTextureLayer, numTextureLayers>    layers;
            std::stack<StackEntry>                          boundTextureStack;
            GLTextureLayer*                                 activeLayerRef      = nullptr;
        };

        struct GLVertexArrayState
        {
            GLuint boundVertexArray         = 0;
            GLuint boundElementArrayBuffer  = 0;
        };

        struct GLShaderState
        {
            GLuint boundProgram = 0;
        };

        struct GLSamplerState
        {
            std::array<GLuint, numTextureLayers> boundSamplers;
        };

    private:

        GLLimits                        limits_;

        OpenGLDependentStateDescriptor  apiDependentState_;

        GLCommonState                   commonState_;
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

        bool                            emulateClipControl_     = false;
        GLint                           renderTargetHeight_     = 0;

        GLDepthStencilState*            boundDepthStencilState_ = nullptr;
        GLRasterizerState*              boundRasterizerState_   = nullptr;
        GLBlendState*                   boundBlendState_        = nullptr;
        bool                            colorMaskOnStack_       = false;

};


} // /namespace LLGL


#endif



// ================================================================================
