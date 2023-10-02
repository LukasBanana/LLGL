/*
 * ExampleBase.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EXAMPLE_BASE_H
#define LLGL_EXAMPLE_BASE_H


#include <LLGL/LLGL.h>
#include <LLGL/Utils/Utility.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/Utils/Parse.h>
#include <LLGL/Container/Strings.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Platform/Platform.h>
#include <Gauss/Gauss.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <map>
#include <type_traits>
#include "GeometryUtils.h"
#include "Stopwatch.h"

#ifdef LLGL_OS_ANDROID
#   include <android_native_app_glue.h>
#   include <android/log.h>
#endif


/*
 * Global helper functions
 */

// Let the user choose a renderer module (using std::cin).
std::string GetSelectedRendererModule(int argc, char* argv[]);

// Load image from file, create texture, upload image into texture, and generate MIP-maps.
LLGL::Texture* LoadTextureWithRenderer(
    LLGL::RenderSystem& renderSys,
    const std::string&  filename,
    long                bindFlags   = (LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment),
    LLGL::Format        format      = LLGL::Format::RGBA8UNorm
);

// Save texture image to a PNG file.
bool SaveTextureWithRenderer(LLGL::RenderSystem& renderSys, LLGL::Texture& texture, const std::string& filename, std::uint32_t mipLevel = 0);


/*
 * Example base class
 */

// Helper structure for examples to organize shaders for a PSO
struct ShaderPipeline
{
    LLGL::Shader* vs = nullptr; // Vertex shader
    LLGL::Shader* hs = nullptr; // Hull shader (aka. tessellation control shader)
    LLGL::Shader* ds = nullptr; // Domain shader (aka. tessellation evaluation shader)
    LLGL::Shader* gs = nullptr; // Geometry shader
    LLGL::Shader* ps = nullptr; // Pixel shader (aka. fragment shader)
    LLGL::Shader* cs = nullptr; // Compute shader
};

class ExampleBase
{

public:

    // Lets the user select a renderer module from the standard input.
    static void SelectRendererModule(int argc, char* argv[]);

    #if defined LLGL_OS_ANDROID
    static void SetAndroidApp(android_app* androidApp);
    #endif

    virtual ~ExampleBase() = default;

    // Runs the main loop.
    void Run();

    // Draws a frame and presents the result on the screen.
    void DrawFrame();

protected:

    struct TutorialShaderDescriptor
    {
        TutorialShaderDescriptor(
            LLGL::ShaderType    type,
            const std::string&  filename
        );

        TutorialShaderDescriptor(
            LLGL::ShaderType    type,
            const std::string&  filename,
            const std::string&  entryPoint,
            const std::string&  profile
        );

        LLGL::ShaderType    type;
        std::string         filename;
        std::string         entryPoint;
        std::string         profile;
    };

private:

    class WindowEventHandler : public LLGL::Window::EventListener
    {

        public:

            WindowEventHandler(ExampleBase& app, LLGL::SwapChain* swapChain, Gs::Matrix4f& projection);

            void OnResize(LLGL::Window& sender, const LLGL::Extent2D& clientAreaSize) override;
            void OnUpdate(LLGL::Window& sender) override;

        private:

            ExampleBase&        app_;
            LLGL::SwapChain*    swapChain_;
            Gs::Matrix4f&       projection_;

    };

    class CanvasEventHandler : public LLGL::Canvas::EventListener
    {

        public:

            CanvasEventHandler(ExampleBase& app, LLGL::SwapChain* swapChain, Gs::Matrix4f& projection);

            void OnDraw(LLGL::Canvas& sender) override;
            void OnResize(LLGL::Canvas& sender, const LLGL::Extent2D& clientAreaSize) override;

        private:

            ExampleBase&        app_;
            LLGL::SwapChain*    swapChain_;
            Gs::Matrix4f&       projection_;

    };

private:

    #ifdef LLGL_OS_ANDROID
    static android_app*                         androidApp_;
    #endif

    std::unique_ptr<LLGL::RenderingDebugger>    debuggerObj_;

    bool                                        loadingDone_        = false;

    static std::string                          rendererModule_;

    std::uint32_t                               samples_            = 1;

protected:

    friend class ResizeEventHandler;

    // Default background color for all tutorials
    const float                                 backgroundColor[4]  = { 0.1f, 0.1f, 0.4f };

    // Render system
    LLGL::RenderSystemPtr                       renderer;

    // Main swap-chain
    LLGL::SwapChain*                            swapChain           = nullptr;

    // Main command buffer
    LLGL::CommandBuffer*                        commands            = nullptr;

    // Command queue
    LLGL::CommandQueue*                         commandQueue        = nullptr;

    // User input event listener
    LLGL::Input                                 input;

    // Primary timer object
    Stopwatch                                   timer;

    // Primary camera projection
    Gs::Matrix4f                                projection;

protected:

    ExampleBase(
        const LLGL::UTF8String& title,
        const LLGL::Extent2D&   resolution  = { 800, 600 },
        std::uint32_t           samples     = 8,
        bool                    vsync       = true,
        bool                    debugger    = true
    );

    // Callback to draw each frame
    virtual void OnDrawFrame() = 0;

    // Callback when the window has been resized. Can also be detected by using a custom window event listener.
    virtual void OnResize(const LLGL::Extent2D& resoluion);

private:

    // Internal function to load a shader.
    LLGL::Shader* LoadShaderInternal(
        const TutorialShaderDescriptor&             shaderDesc,
        const LLGL::ArrayView<LLGL::VertexFormat>&  vertexFormats,
        const LLGL::VertexFormat&                   streamOutputFormat,
        const std::vector<LLGL::FragmentAttribute>& fragmentAttribs,
        const LLGL::ShaderMacro*                    defines,
        bool                                        patchClippingOrigin
    );

protected:

    // Loads a shader from file with optional vertex formats and stream-output format.
    LLGL::Shader* LoadShader(
        const TutorialShaderDescriptor&             shaderDesc,
        const LLGL::ArrayView<LLGL::VertexFormat>&  vertexFormats       = {},
        const LLGL::VertexFormat&                   streamOutputFormat  = {},
        const LLGL::ShaderMacro*                    defines             = nullptr
    );

    // Loads a shader from file with fragment attributes.
    LLGL::Shader* LoadShader(
        const TutorialShaderDescriptor&             shaderDesc,
        const std::vector<LLGL::FragmentAttribute>& fragmentAttribs,
        const LLGL::ShaderMacro*                    defines             = nullptr
    );

    // Load a shader from file and adds 'PatchClippingOrigin' to the compile flags if the screen origin is lower-left; see IsScreenOriginLowerLeft().
    LLGL::Shader* LoadShaderAndPatchClippingOrigin(
        const TutorialShaderDescriptor&             shaderDesc,
        const LLGL::ArrayView<LLGL::VertexFormat>&  vertexFormats       = {},
        const LLGL::VertexFormat&                   streamOutputFormat  = {},
        const LLGL::ShaderMacro*                    defines             = nullptr
    );

    // Loads a vertex shader with standard filename convention.
    LLGL::Shader* LoadStandardVertexShader(
        const char*                                 entryPoint      = "VS",
        const LLGL::ArrayView<LLGL::VertexFormat>&  vertexFormats   = {},
        const LLGL::ShaderMacro*                    defines         = nullptr);

    // Loads a fragment shader with standard filename convention.
    LLGL::Shader* LoadStandardFragmentShader(
        const char*                                 entryPoint      = "PS",
        const std::vector<LLGL::FragmentAttribute>& fragmentAttribs = {},
        const LLGL::ShaderMacro*                    defines         = nullptr
    );

    // Loads a compute shader with standard filename convention.
    LLGL::Shader* LoadStandardComputeShader(
        const char*                 entryPoint  = "CS",
        const LLGL::ShaderMacro*    defines     = nullptr
    );

    // Loads a shader pipeline with vertex and fragment shaders and with standard filename convention.
    ShaderPipeline LoadStandardShaderPipeline(const std::vector<LLGL::VertexFormat>& vertexFormats);

    // Throws an exception if the specified PSO creation failed.
    void ThrowIfFailed(LLGL::PipelineState* pso);

    // Load image from file, create texture, upload image into texture, and generate MIP-maps.
    LLGL::Texture* LoadTexture(
        const std::string&  filename,
        long                bindFlags   = (LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment),
        LLGL::Format        format      = LLGL::Format::RGBA8UNorm
    );

    // Save texture image to a PNG file.
    bool SaveTexture(LLGL::Texture& texture, const std::string& filename, std::uint32_t mipLevel = 0);

    // Captures the current framebuffer into a new texture.
    LLGL::Texture* CaptureFramebuffer(LLGL::CommandBuffer& commandBuffer, const LLGL::RenderTarget* resolutionSource = nullptr);

    // Returns the aspect ratio of the swap-chain resolution (X:Y).
    float GetAspectRatio() const;

    // Returns ture if OpenGL is used as rendering API.
    bool IsOpenGL() const;

    // Returns ture if Vulkan is used as rendering API.
    bool IsVulkan() const;

    // Returns ture if Direct3D is used as rendering API.
    bool IsDirect3D() const;

    // Returns ture if Metal is used as rendering API.
    bool IsMetal() const;

    // Used by the window resize handler
    bool IsLoadingDone() const;

    // Returns true if the screen origin of the selected renderer is lower-left. See RenderingCapabilities::screenOrigin.
    bool IsScreenOriginLowerLeft() const;

    // Returns a perspective projection with the specified parameters for the respective renderer.
    Gs::Matrix4f PerspectiveProjection(float aspectRatio, float near, float far, float fov);

    // Returns an orthogonal projection with the speciifed parameters for the respective renderer.
    Gs::Matrix4f OrthogonalProjection(float width, float height, float near, float far);

    // Returns true if the specified shading language is supported.
    bool Supported(const LLGL::ShadingLanguage shadingLanguage) const;

    // Returns the number of samples that was used when the swap-chain was created.
    inline std::uint32_t GetSampleCount() const
    {
        return samples_;
    }

protected:

    // Returns the name of the renderer module (e.g. "OpenGL" or "Direct3D11").
    static const std::string& GetModuleName();

protected:

    template <typename Container>
    std::size_t GetArraySize(const Container& container) const
    {
        return (container.size() * sizeof(typename Container::value_type));
    }

    template <typename T, std::size_t N>
    std::size_t GetArraySize(const T (&container)[N]) const
    {
        return (N * sizeof(T));
    }

    template <typename Container>
    LLGL::Buffer* CreateVertexBuffer(const Container& vertices, const LLGL::VertexFormat& vertexFormat)
    {
        return renderer->CreateBuffer(
            LLGL::VertexBufferDesc(GetArraySize(vertices), vertexFormat),
            &vertices[0]
        );
    }

    template <typename Container>
    LLGL::Buffer* CreateIndexBuffer(const Container& indices, const LLGL::Format format)
    {
        return renderer->CreateBuffer(
            LLGL::IndexBufferDesc(GetArraySize(indices), format),
            &indices[0]
        );
    }

    template <typename T>
    LLGL::Buffer* CreateConstantBuffer(const T& initialData)
    {
        static_assert(!std::is_pointer<T>::value, "buffer type must not be a pointer");
        return renderer->CreateBuffer(
            LLGL::ConstantBufferDesc(sizeof(T)),
            &initialData
        );
    }

};


#if defined LLGL_OS_ANDROID

#define LLGL_ANDROID_STDERR(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "threaded_app", __VA_ARGS__))

template <typename T>
void RunExample(android_app* state)
{
    try
    {
        ExampleBase::SetAndroidApp(state);
        auto tutorial = std::unique_ptr<T>(new T());
        tutorial->Run();
    }
    catch (const std::exception& e)
    {
        LLGL_ANDROID_STDERR("%s\n", e.what());
    }
}

#define LLGL_IMPLEMENT_EXAMPLE(CLASS)       \
    void android_main(android_app* state)   \
    {                                       \
        return RunExample<CLASS>(state);    \
    }

#elif defined LLGL_OS_IOS

extern std::unique_ptr<ExampleBase> InstantiateExample();

#define LLGL_IMPLEMENT_EXAMPLE(CLASS)                       \
    std::unique_ptr<ExampleBase> InstantiateExample()       \
    {                                                       \
        return std::unique_ptr<ExampleBase>(new CLASS());   \
    }

#else // LLGL_OS_*

template <typename T>
int RunExample(int argc, char* argv[])
{
    try
    {
        ExampleBase::SelectRendererModule(argc, argv);
        auto tutorial = std::unique_ptr<T>(new T());
        tutorial->Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        #ifdef _WIN32
        system("pause");
        #endif
    }
    return 0;
}

#define LLGL_IMPLEMENT_EXAMPLE(CLASS)           \
    int main(int argc, char* argv[])            \
    {                                           \
        return RunExample<CLASS>(argc, argv);   \
    }

#endif // /LLGL_OS_*


#endif

