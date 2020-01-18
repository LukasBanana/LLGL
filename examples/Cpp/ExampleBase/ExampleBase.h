/*
 * ExampleBase.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_EXAMPLE_BASE_H
#define LLGL_EXAMPLE_BASE_H


#include <LLGL/LLGL.h>
#include <LLGL/Utility.h>
#include <LLGL/Platform/Platform.h>
#include <Gauss/Gauss.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <map>
#include <type_traits>
#include "GeometryUtility.h"

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

    class ResizeEventHandler : public LLGL::Window::EventListener
    {

        public:

            ResizeEventHandler(ExampleBase& tutorial, LLGL::RenderContext* context, Gs::Matrix4f& projection);

            void OnResize(LLGL::Window& sender, const LLGL::Extent2D& clientAreaSize) override;
            void OnTimer(LLGL::Window& sender, std::uint32_t timerID) override;

        private:

            ExampleBase&            tutorial_;
            LLGL::RenderContext*    context_;
            Gs::Matrix4f&           projection_;

    };

    struct ShaderProgramRecall
    {
        std::vector<TutorialShaderDescriptor>   shaderDescs;
        std::vector<LLGL::Shader*>              shaders;
        LLGL::VertexShaderAttributes            vertexAttribs;
        LLGL::FragmentShaderAttributes          fragmentAttribs;
    };

private:

    #ifdef LLGL_OS_ANDROID
    static android_app*                         androidApp_;
    #endif

    std::unique_ptr<LLGL::RenderingProfiler>    profilerObj_;
    std::unique_ptr<LLGL::RenderingDebugger>    debuggerObj_;

    std::map< LLGL::ShaderProgram*,
              ShaderProgramRecall >             shaderPrograms_;

    bool                                        loadingDone_        = false;

    static std::string                          rendererModule_;

    std::uint32_t                               samples_            = 1;

protected:

    friend class ResizeEventHandler;

    // Default background color for all tutorials
    const LLGL::ColorRGBAf                      backgroundColor = { 0.1f, 0.1f, 0.4f };

    // Render system
    std::unique_ptr<LLGL::RenderSystem>         renderer;

    // Main render context
    LLGL::RenderContext*                        context         = nullptr;

    // Main command buffer
    LLGL::CommandBuffer*                        commands        = nullptr;

    // Command queue
    LLGL::CommandQueue*                         commandQueue    = nullptr;

    // User input event listener
    std::shared_ptr<LLGL::Input>                input;

    // Primary timer object
    std::unique_ptr<LLGL::Timer>                timer;

    // Rendering profiler (read only)
    const LLGL::RenderingProfiler&              profiler;

    // Primary camera projection
    Gs::Matrix4f                                projection;

protected:

    ExampleBase(
        const std::wstring&     title,
        const LLGL::Extent2D&   resolution  = { 800, 600 },
        std::uint32_t           samples     = 8,
        bool                    vsync       = true,
        bool                    debugger    = true
    );

    // Callback to draw each frame
    virtual void OnDrawFrame() = 0;

    // Callback when the window has been resized. Can also be detected by using a custom window event listener.
    virtual void OnResize(const LLGL::Extent2D& resoluion);

protected:

    // Creats a shader program and loads all specified shaders from file.
    LLGL::ShaderProgram* LoadShaderProgram(
        const std::vector<TutorialShaderDescriptor>&    shaderDescs,
        const std::vector<LLGL::VertexFormat>&          vertexFormats       = {},
        const LLGL::VertexFormat&                       streamOutputFormat  = {},
        const std::vector<LLGL::FragmentAttribute>&     fragmentAttribs     = {},
        const LLGL::ShaderMacro*                        defines             = nullptr
    );

    // Reloads the specified shader program from the previously specified shader source files.
    bool ReloadShaderProgram(LLGL::ShaderProgram*& shaderProgram);

    // Load standard shader program (with vertex- and fragment shaders).
    LLGL::ShaderProgram* LoadStandardShaderProgram(const std::vector<LLGL::VertexFormat>& vertexFormats);

    // Load image from file, create texture, upload image into texture, and generate MIP-maps.
    LLGL::Texture* LoadTexture(
        const std::string&  filename,
        long                bindFlags   = (LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment),
        LLGL::Format        format      = LLGL::Format::RGBA8UNorm
    );

    // Save texture image to a PNG file.
    bool SaveTexture(LLGL::Texture& texture, const std::string& filename, std::uint32_t mipLevel = 0);

    // Returns the aspect ratio of the render context resolution (X:Y).
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

    // Returns a perspective projection with the specified parameters for the respective renderer.
    Gs::Matrix4f PerspectiveProjection(float aspectRatio, float near, float far, float fov);

    // Returns an orthogonal projection with the speciifed parameters for the respective renderer.
    Gs::Matrix4f OrthogonalProjection(float width, float height, float near, float far);

    // Returns true if the specified shading language is supported.
    bool Supported(const LLGL::ShadingLanguage shadingLanguage) const;

    // Returns the number of samples that was used when the render context was created.
    inline std::uint32_t GetSampleCount() const
    {
        return samples_;
    }

protected:

    // Returns the name of the renderer module (e.g. "OpenGL" or "Direct3D11").
    static const std::string& GetModuleName();

protected:

    template <typename VertexType>
    LLGL::Buffer* CreateVertexBuffer(const std::vector<VertexType>& vertices, const LLGL::VertexFormat& vertexFormat)
    {
        return renderer->CreateBuffer(
            LLGL::VertexBufferDesc(static_cast<std::uint32_t>(vertices.size() * sizeof(VertexType)), vertexFormat),
            vertices.data()
        );
    }

    template <typename IndexType>
    LLGL::Buffer* CreateIndexBuffer(const std::vector<IndexType>& indices, const LLGL::Format format)
    {
        return renderer->CreateBuffer(
            LLGL::IndexBufferDesc(static_cast<std::uint32_t>(indices.size() * sizeof(IndexType)), format),
            indices.data()
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

