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
#include <array>
#include <vector>
#include <stack>


namespace LLGL
{


class GLStateManager
{

    public:

        /* ----- Common ----- */

        GLStateManager();

        static GLStateManager* active;

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

        void SetDepthFunc(GLenum func);
        void SetStencilFunc(GLenum face, const GLStencil& state);
        void SetViewports(const std::vector<GLViewport>& viewports);
        void SetDepthRanges(const std::vector<GLDepthRange>& depthRanges);
        void SetScissors(const std::vector<GLScissor>& scissors);

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

        /* ----- Texture binding ----- */

        void ActiveTexture(unsigned int layer);

        void BindTexture(GLTextureTarget target, GLuint texture);
        void ForcedBindTexture(GLTextureTarget target, GLuint texture);
        
        void PushBoundTexture(unsigned int layer, GLTextureTarget target);
        void PopBoundTexture();

        void BindTexture(const GLTexture& texture);
        void ForcedBindTexture(const GLTexture& texture);

        /* ----- Shader binding ----- */

        void BindShaderProgram(GLuint program);

    private:

        void SetStencilFunc(GLenum face, GLStencil& to, const GLStencil& from);

        static const std::size_t numTextureLayers   = 32;
        static const std::size_t numStates          = (static_cast<std::size_t>(GLState::PROGRAM_POINT_SIZE) + 1);
        static const std::size_t numBufferTargets   = (static_cast<std::size_t>(GLBufferTarget::UNIFORM_BUFFER) + 1);
        static const std::size_t numTextureTargets  = (static_cast<std::size_t>(GLTextureTarget::TEXTURE_2D_MULTISAMPLE_ARRAY) + 1);

        struct GLCommonState
        {
            GLenum                      depthFunc    = GL_LESS;
            GLStencil                   stencil[2];
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
            GLuint boundProgram = 0;
        };

        GLCommonState               commonState_;
        GLRenderState               renderState_;
        GLBufferState               bufferState_;
        GLTextureState              textureState_;
        GLShaderState               shaderState_;

        GLTextureLayer*             activeTextureLayer_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
