/*
 * GLStateManager.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_STATE_MANAGER_H
#define LLGL_GL_STATE_MANAGER_H


#include "GLState.h"
#include "GLContextState.h"
#include <LLGL/TextureFlags.h>
#include <LLGL/CommandBufferFlags.h>
#include "../OpenGL.h"
#include "../../../Core/Assertion.h"
#include <stack>
#include <cstdint>


namespace LLGL
{


class RenderTarget;
class RenderPass;
class GLContext;
class GLRenderTarget;
class GLSwapChain;
class GLBuffer;
class GLBufferWithXFB;
class GLTexture;
class GLDepthStencilState;
class GLRasterizerState;
class GLBlendState;
class GLRenderPass;
class GLProgramPipeline;
class GLShaderProgram;
class GLEmulatedSampler;

// OpenGL state machine manager that keeps track of certain GL states.
class GLStateManager
{

    public:

        // Maximal number of resource slots.
        static const std::uint32_t g_maxNumResourceSlots = 64;

    public:

        // GL limitations required for validation of state parameters
        struct GLLimits
        {
            GLint       maxViewports        = 0;                // Maximum number of viewports (minimum value is 16).
            GLfloat     lineWidthRange[2]   = { 1.0f, 1.0f };   // Minimal range of both <aliased> and <smooth> line width range.
            GLint       maxDebugNameLength  = 0;                // Maximal length of names for debug groups (minimum value is 1).
            GLint       maxDebugStackDepth  = 0;                // Maximal depth of the debug group stack (minimum value is 64).
            GLint       maxLabelLength      = 0;                // Maximal length of debug labels (minimum value is 256).
            GLuint      maxTextureLayers    = 0;                // Maximal number of texture layers (minimum value is 16).
            GLuint      maxImageUnits       = 0;                // Maximal number of image units.
        };

    public:

        /* ----- Common ----- */

        GLStateManager();
        ~GLStateManager();

        // Returns the active GL state manager.
        static inline GLStateManager& Get()
        {
            LLGL_DEBUG_ASSERT_PTR(current_);
            return *current_;
        }

        // Makes the state manager of the specified GL context the current. This should only be called inside GLContext::SetCurrent().
        static void SetCurrentFromGLContext(GLContext& context);

        // Queries all supported and available GL extensions and limitations, then stores it internally (must be called once a GL context has been created).
        void DetermineExtensionsAndLimits();

        //TODO: viewports and scissors must be updated!
        // Notifies the state manager about a new render-target height.
        void ResetFramebufferHeight(GLint height);

        /* ----- Boolean states ----- */

        static GLenum GetGLCapability(GLState state);

        // Clears the cache by querying all states from OpenGL.
        void ClearCache();

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

        void SetViewport(const GLViewport& viewport);
        void SetViewportArray(GLuint first, GLsizei count, const GLViewport* viewports);

        void SetDepthRange(const GLDepthRange& depthRange);
        void SetDepthRangeArray(GLuint first, GLsizei count, const GLDepthRange* depthRanges);

        void SetScissor(const GLScissor& scissor);
        void SetScissorArray(GLuint first, GLsizei count, const GLScissor* scissors);

        void SetClipControl(GLenum origin, GLenum depth);
        void SetPolygonMode(GLenum mode);
        void SetPolygonOffset(GLfloat factor, GLfloat units, GLfloat clamp);
        void SetCullFace(GLenum face);
        void SetFrontFace(GLenum mode);
        void SetPatchVertices(GLint patchVertices);
        void SetLineWidth(GLfloat width);
        void SetPrimitiveRestartIndex(GLuint index);
        #if 0//TODO
        //void SetSampleMask(GLuint maskNumber, GLbitfield mask);
        #endif

        void SetPixelStorePack(GLint rowLength, GLint imageHeight, GLint alignment);
        void SetPixelStoreUnpack(GLint rowLength, GLint imageHeight, GLint alignment);

        /* ----- Depth-stencil states ----- */

        void NotifyDepthStencilStateRelease(GLDepthStencilState* depthStencilState);

        void BindDepthStencilState(GLDepthStencilState* depthStencilState);

        void SetDepthFunc(GLenum func);
        void SetDepthMask(GLboolean flag);
        void SetStencilRef(GLint ref, GLenum face);

        /* ----- Rasterizer states ----- */

        void NotifyRasterizerStateRelease(GLRasterizerState* rasterizerState);

        void BindRasterizerState(GLRasterizerState* rasterizerState);

        /* ----- Blend states ----- */

        void NotifyBlendStateRelease(GLBlendState* blendState);

        void BindBlendState(GLBlendState* blendState);

        void SetBlendColor(const GLfloat color[4]);
        void SetLogicOp(GLenum opcode);

        /* ----- Buffer ----- */

        static GLenum ToGLBufferTarget(GLBufferTarget target);

        void BindBuffer(GLBufferTarget target, GLuint buffer);
        void BindBufferBase(GLBufferTarget target, GLuint index, GLuint buffer);
        void BindBuffersBase(GLBufferTarget target, GLuint first, GLsizei count, const GLuint* buffers);
        void BindBufferRange(GLBufferTarget target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
        void BindBuffersRange(GLBufferTarget target, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizeiptr* sizes);

        void BindVertexArray(GLuint vertexArray);

        void BindGLBuffer(const GLBuffer& buffer);

        void NotifyVertexArrayRelease(GLuint vertexArray);

        /**
        \brief Binds the specified GL_ELEMENT_ARRAY_BUFFER (i.e. index buffer) to the next VAO (or the current one).
        \see BindVertexArray
        */
        void BindElementArrayBufferToVAO(GLuint buffer, bool indexType16Bits);

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

        void PushBoundRenderbuffer();
        void PopBoundRenderbuffer();

        void DeleteRenderbuffer(GLuint renderbuffer);

        /* ----- Texture ----- */

        static GLTextureTarget GetTextureTarget(const TextureType type);

        // Returns GL_TEXTURE0..GL_TEXTURE31
        static GLenum ToGLTextureLayer(GLuint layer);

        // Returns GL_TEXTURE_2D etc.
        static GLenum ToGLTextureTarget(const GLTextureTarget target);

        void ActiveTexture(GLuint layer);

        void BindTexture(GLTextureTarget target, GLuint texture);
        void BindTextures(GLuint first, GLsizei count, const GLTextureTarget* targets, const GLuint* textures);
        void UnbindTextures(GLuint first, GLsizei count);

        void BindImageTexture(GLuint unit, GLint level, GLenum format, GLuint texture);
        void BindImageTextures(GLuint first, GLsizei count, const GLenum* formats, const GLuint* textures);
        void UnbindImageTextures(GLuint first, GLsizei count);

        void PushBoundTexture(GLuint layer, GLTextureTarget target);
        void PushBoundTexture(GLTextureTarget target);
        void PopBoundTexture();

        void BindGLTexture(GLTexture& texture);

        void DeleteTexture(GLuint texture, GLTextureTarget target, bool invalidateActiveLayerOnly = false);

        /* ----- Sampler ----- */

        void BindSampler(GLuint layer, GLuint sampler);
        void BindSamplers(GLuint first, GLsizei count, const GLuint* samplers);

        void NotifySamplerRelease(GLuint sampler);

        void BindEmulatedSampler(GLuint layer, const GLEmulatedSampler& sampler);
        void BindCombinedEmulatedSampler(GLuint layer, const GLEmulatedSampler& sampler, GLTexture& texture);

        /* ----- Shader program ----- */

        void BindShaderProgram(GLuint program);

        void PushBoundShaderProgram();
        void PopBoundShaderProgram();

        void NotifyShaderProgramRelease(GLShaderProgram* shaderProgram);

        GLuint GetBoundShaderProgram() const;

        /* ----- Program pipeline ----- */

        void BindProgramPipeline(GLuint pipeline);

        void NotifyProgramPipelineRelease(GLProgramPipeline* programPipeline);

        GLuint GetBoundProgramPipeline() const;

        /* ----- Render pass ----- */

        void BindRenderTarget(RenderTarget& renderTarget, GLStateManager** nextStateManager = nullptr);

        void ClearAttachmentsWithRenderPass(
            const GLRenderPass& renderPassGL,
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues
        );

        void Clear(long flags);
        void ClearBuffers(std::uint32_t numAttachments, const AttachmentClear* attachments);

        /* ----- Transform feedback ----- */

        void BindTransformFeedback(GLuint transformFeedback);

        void NotifyTransformFeedbackRelease(GLBufferWithXFB* bufferWithXfb);

        /* ----- Feedback ----- */

        // Returns the limitations for this GL context.
        inline const GLLimits& GetLimits() const
        {
            return limits_;
        }

        // Returns the height of the current framebuffer.
        inline GLint GetFramebufferHeight() const
        {
            return framebufferHeight_;
        }

    public:

        // Returns the common denominator of limitations for all GL contexts.
        static inline const GLLimits& GetCommonLimits()
        {
            return commonLimits_;
        }

    private:

        struct GLFramebufferClearState;

    private:

        bool NeedsAdjustedViewport() const;

        void AdjustViewport(GLViewport& outViewport, const GLViewport& inViewport);
        void AdjustScissor(GLScissor& outScissor, const GLScissor& inScissor);

        void AssertViewportLimit(GLuint first, GLsizei count);

        GLContextState::TextureLayer* GetActiveTextureLayer();
        void NotifyTextureRelease(GLuint texture, GLTextureTarget target, bool invalidateActiveLayerOnly);

        void SetFrontFaceInternal(GLenum mode);
        void FlipFrontFacing(bool isFlipped);

        void DetermineLimits();

        #ifdef LLGL_GL_ENABLE_VENDOR_EXT
        void DetermineVendorSpecificExtensions();
        #endif

        /* ----- Stacks ----- */

        void PrepareRasterizerStateForClear(GLFramebufferClearState& clearState);
        void PrepareColorMaskForClear(GLFramebufferClearState& clearState);
        void PrepareDepthMaskForClear(GLFramebufferClearState& clearState);
        void PrepareStencilMaskForClear(GLFramebufferClearState& clearState);
        void RestoreClearState(const GLFramebufferClearState& clearState);

        /* ----- Render pass ----- */

        std::uint32_t ClearColorBuffers(
            const std::uint8_t*         colorBuffers,
            std::uint32_t               numClearValues,
            const ClearValue*           clearValues,
            const ClearValue&           defaultClearValue,
            GLFramebufferClearState&    clearState
        );

    private:

        struct CapabilityStackEntry
        {
            GLState state;
            bool    enabled;
        };

        struct BufferStackEntry
        {
            GLBufferTarget  target;
            GLuint          buffer;
        };

        struct TextureStackEntry
        {
            GLuint          layer;
            GLTextureTarget target;
            GLuint          texture;
        };

        struct FramebufferStackEntry
        {
            GLFramebufferTarget target;
            GLuint              framebuffer;
        };

        struct RenderbufferStackEntry
        {
            GLuint renderbuffer;
        };

        struct ShaderProgramStackEntry
        {
            GLuint program;
        };

    private:

        static GLStateManager*  current_;
        static GLLimits         commonLimits_;          // Common denominator of limitations for all GL contexts

    private:

        GLLimits                            limits_;                // Limitations of this GL context

        GLContextState                      contextState_;

        GLTexture*                          boundGLTextures_[GLContextState::numTextureLayers]          = {};
        const GLEmulatedSampler*            boundGLEmulatedSamplers_[GLContextState::numTextureLayers]  = {};

        GLRenderTarget*                     boundRenderTarget_          = nullptr;

        bool                                indexType16Bits_            = false;
        GLuint                              lastVertexAttribArray_      = 0;

        GLenum                              frontFaceInternal_          = GL_CCW; // actual front face input (without possible inversion)

        bool                                flipViewportYPos_           = false;
        bool                                flipFrontFacing_            = false;
        bool                                emulateOriginUpperLeft_     = false;
        bool                                emulateDepthModeZeroToOne_  = false;
        GLint                               framebufferHeight_          = 0;

        GLDepthStencilState*                boundDepthStencilState_     = nullptr;
        GLRasterizerState*                  boundRasterizerState_       = nullptr;
        GLBlendState*                       boundBlendState_            = nullptr;

        bool                                frontFacingDirtyBit_        = false;

        std::stack<CapabilityStackEntry>    capabilitiesStack_;
        std::stack<BufferStackEntry>        bufferStack_;
        std::stack<TextureStackEntry>       textureState_;
        std::stack<FramebufferStackEntry>   framebufferStack_;
        std::stack<RenderbufferStackEntry>  renderbufferStack_;
        std::stack<ShaderProgramStackEntry> shaderProgramStack_;

};


} // /namespace LLGL


#endif



// ================================================================================
