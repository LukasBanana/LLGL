/*
 * GLContextManager.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_CONTEXT_MANAGER_H
#define LLGL_GL_CONTEXT_MANAGER_H


#include "GLContext.h"
#include <LLGL/RendererConfiguration.h>
#include <LLGL/Container/DynamicArray.h>
#include <vector>
#include <memory>
#include <functional>


namespace LLGL
{

namespace OpenGL
{

struct RenderSystemNativeHandle;

} // /namespace OpenGL


class GLStateManager;

// Helper class to reuse GL contexts for suitable pixel formats.
class GLContextManager
{

    public:

        // Callback interface when a new GLContext is created.
        using NewGLContextCallback = std::function<void(GLContext& context, const GLPixelFormat& pixelFormat)>;

    public:

        GLContextManager(const GLContextManager&) = delete;
        GLContextManager& operator = (const GLContextManager&) = delete;

        // Initializes the context manager and creates the primary GL context.
        GLContextManager(
            const RendererConfigurationOpenGL&  profile,
            const NewGLContextCallback&         newContextCallback      = nullptr,
            const void*                         customNativeHandle      = nullptr,
            std::size_t                         customNativeHandleSize  = 0
        );

    public:

        // Returns a GL context with the specified pixel format or any context if 'pixelFormat' is null.
        std::shared_ptr<GLContext> AllocContext(
            const GLPixelFormat*    pixelFormat             = nullptr,
            bool                    acceptCompatibleFormat  = false,
            Surface*                surface                 = nullptr
        );

    public:

        // Returns the OpenGL profile configuration.
        inline const RendererConfigurationOpenGL& GetProfile() const
        {
            return profile_;
        }

    private:

        struct GLPixelFormatWithContext
        {
            GLPixelFormat               pixelFormat;
            std::unique_ptr<Surface>    surface;
            std::shared_ptr<GLContext>  context;
        };

    private:

        // Creates an invisible surface as placeholder for a GL context.
        std::unique_ptr<Surface> CreatePlaceholderSurface();

        // Makes a new GL context with the specified pixel format and creates a placeholder surface is none was specified.
        std::shared_ptr<GLContext> MakeContextWithPixelFormat(const GLPixelFormat& pixelFormat, Surface* surface = nullptr);

        // Returns a GL context with the specified pixel format or creates a new one if no suitable context could be found.
        std::shared_ptr<GLContext> FindOrMakeContextWithPixelFormat(const GLPixelFormat& pixelFormat, bool acceptCompatibleFormat, Surface* surface = nullptr);

        // Returns any GL context or creates a new one if none has been created yet.
        std::shared_ptr<GLContext> FindOrMakeAnyContext();

        // Initializes the default render states for the specified GL state manager.
        void InitRenderStates(GLStateManager& stateMngr);

    private:

        RendererConfigurationOpenGL             profile_;
        std::vector<GLPixelFormatWithContext>   pixelFormats_;
        DynamicByteArray                        customNativeHandle_;
        NewGLContextCallback                    newContextCallback_;

};


} // /namespace LLGL


#endif



// ================================================================================
