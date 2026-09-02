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
#include <LLGL/Trap.h>
#include <Gauss/Gauss.h>
#include <vector>
#include <random>
#include <map>
#include <type_traits>
#include "GeometryUtils.h"

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

class TrackballRotationModel
{
    LLGL::Offset2D  cursorStartPosition_;
    Gs::Vector3f    cursorStartVector_;
    Gs::Quaternionf modelStartRotation_;

public:
    void Rotate(
        Gs::Quaternionf&        rotation,
        const LLGL::Viewport&   viewport,
        const LLGL::Offset2D&   cursorPosition,
        bool                    isStartPosition = false,
        float                   projZAxis       = 1.0f
    );

};

class ExampleBase
{

public:

    // Lets the user select a renderer module from the standard input.
    static void ParseProgramArgs(int argc, char* argv[]);

    #if defined LLGL_OS_ANDROID
    static void SetAndroidApp(android_app* androidApp);
    static android_app* GetAndroidApp();
    #endif

    virtual ~ExampleBase() = default;

    // Runs the main loop.
    int Run();

    // Draws a frame and presents the result on the screen.
    void DrawFrame();

    // Resizes the swap-chain and notifies the app about the resize.
    void Resize(const LLGL::Extent2D& clientAreaSize);

    // Returns true if the app can currently draw a frame. Otherwise, the window is resized too small for instance.
    bool IsDrawable() const;

protected:

    struct ShaderDescWrapper
    {
        ShaderDescWrapper(LLGL::ShaderType type, const char* filename);
        ShaderDescWrapper(LLGL::ShaderType type, const char* filename, const char* entryPoint, const char* profile);

        LLGL::ShaderType    type        = LLGL::ShaderType::Undefined;
        const char*         filename    = nullptr;
        const char*         entryPoint  = nullptr;
        const char*         profile     = nullptr;
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

    using RenderingDebuggerPtr = std::unique_ptr<LLGL::RenderingDebugger>;

    #ifdef LLGL_OS_ANDROID
    static android_app*         androidApp_;
    #endif

    RenderingDebuggerPtr        debuggerObj_;

    std::uint32_t               samples_            = 1;
    LLGL::Extent2D              initialResolution_;
    LLGL::Extent2D              drawableSize_;
    bool                        showTimeRecords_    = false;
    bool                        fullscreen_         = false;
    bool                        useRightHandedProj_ = false;
    int                         returnCode_         = 0;
    std::uint64_t               lastFrameTick_      = 0;

    TrackballRotationModel      trackballRotation_;

    struct ShaderModelInfo
    {
        std::string minHLSLShaderModel  = "5_0";
        std::string minMetalShaderModel = "1.1";
        std::string intermediateHLSLProfile;
    }
    shaderModelInfo_;

protected:

    friend class ResizeEventHandler;

    // Default background color for all tutorials
    const float                 backgroundColor[4]  = { 0.1f, 0.1f, 0.4f, 1.0f };

    // Render system
    LLGL::RenderSystemPtr       renderer;

    // Main swap-chain
    LLGL::SwapChain*            swapChain           = nullptr;

    // Main command buffer
    LLGL::CommandBuffer*        commands            = nullptr;

    // If Tier1 is supported, this points to the same command buffer as `commands`. Otherwise, null.
    LLGL::CommandBufferTier1*   commandsTier1       = nullptr;

    // Command queue
    LLGL::CommandQueue*         commandQueue        = nullptr;

    // User input event listener
    LLGL::Input                 input;

    // Primary camera projection
    Gs::Matrix4f                projection;

protected:

    ExampleBase(const LLGL::UTF8String& title);

    // Callback to draw each frame
    virtual void OnDrawFrame(float deltaTime) = 0;

    // Callback when the window has been resized. Can also be detected by using a custom window event listener.
    virtual void OnResize(const LLGL::Extent2D& resolution);

private:

    static void MainLoopWrapper(void* args);

    // Internal main loop. This is called manually on most platforms. With WebAssembly, it's passed to the browser glue code.
    void MainLoop();

    // Internal function to load a shader.
    LLGL::Shader* LoadShaderInternal(const ShaderDescWrapper& shaderDesc, const LLGL::ShaderMacro* defines, long compileFlags);

    struct ShaderTargetInfo
    {
        LLGL::ShadingLanguage               targetLanguage;
        const char*                         profile;
        std::initializer_list<const char*>  suffixes;
    };

    LLGL::Shader* LoadShaderForTargetLanguage(
        LLGL::ShaderType                                type,
        const char*                                     basename,
        const char*                                     entryPoint,
        const LLGL::ShaderMacro*                        defines,
        long                                            compileFlags,
        const std::initializer_list<ShaderTargetInfo>&  targetInfos
    );

protected:

    // Sets the minimum required shader model for the target platform.
    // If unsupported, the function initiates to exit the application and returns false.
    bool MinimumShaderModel(const char* hlslVersion = "5.0", const char* glslVersion = "150", const char* esslVersion = "300", const char* metalVersion = "1.1");

    // Loads a shader from file with optional vertex formats and stream-output format.
    LLGL::Shader* LoadShader(const ShaderDescWrapper& shaderDesc, const LLGL::ShaderMacro* defines = nullptr);

    // Load a shader from file and adds 'PatchClippingOrigin' to the compile flags if the screen origin is lower-left; see IsScreenOriginLowerLeft().
    LLGL::Shader* LoadShaderAndPatchClippingOrigin(const ShaderDescWrapper& shaderDesc, const LLGL::ShaderMacro* defines = nullptr);

    // Loads a vertex/fragment/compute shader with standard filename convention.
    LLGL::Shader* LoadVertexShader(const char* basename, const char* entryPoint = "VS", const LLGL::ShaderMacro* defines = nullptr, long compileFlags = 0);
    LLGL::Shader* LoadTessControlShader(const char* basename, const char* entryPoint = "HS", const LLGL::ShaderMacro* defines = nullptr, long compileFlags = 0);
    LLGL::Shader* LoadTessEvaluationShader(const char* basename, const char* entryPoint = "DS", const LLGL::ShaderMacro* defines = nullptr, long compileFlags = 0);
    LLGL::Shader* LoadGeometryShader(const char* basename, const char* entryPoint = "GS", const LLGL::ShaderMacro* defines = nullptr, long compileFlags = 0);
    LLGL::Shader* LoadFragmentShader(const char* basename, const char* entryPoint = "PS", const LLGL::ShaderMacro* defines = nullptr, long compileFlags = 0);
    LLGL::Shader* LoadComputeShader(const char* basename, const char* entryPoint = "CS", const LLGL::ShaderMacro* defines = nullptr, long compileFlags = 0);

    inline LLGL::Shader* LoadStandardVertexShader(const char* entryPoint = "VS", const LLGL::ShaderMacro* defines = nullptr)
    {
        return LoadVertexShader("Example", entryPoint, defines);
    }

    inline LLGL::Shader* LoadStandardFragmentShader(const char* entryPoint = "PS", const LLGL::ShaderMacro* defines = nullptr)
    {
        return LoadFragmentShader("Example", entryPoint, defines);
    }

    inline LLGL::Shader* LoadStandardComputeShader(const char* entryPoint = "CS", const LLGL::ShaderMacro* defines = nullptr)
    {
        return LoadComputeShader("Example", entryPoint, defines);
    }

    // Loads a shader pipeline with vertex and fragment shaders and with standard filename convention.
    ShaderPipeline LoadStandardShaderPipeline();

    // Throws an exception if the specified PSO creation failed.
    bool ReportPSOErrors(const LLGL::PipelineState* pso);

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

    // Loads a 3D model from file and determines the coordinates depending on the current projection matrix.
    TriangleMesh Load3DModel(std::vector<TexturedVertex>& vertices, const std::string& filename, unsigned verticesPerFace = 3);
    std::vector<TexturedVertex> Load3DModel(const std::string& filename, unsigned verticesPerFace = 3);

    // Returns the aspect ratio of the swap-chain resolution (X:Y).
    float GetAspectRatio() const;

    // Returns the flags to generate a projection matrix for this renderer.
    int GetProjectionMatrixFlags() const;

    // Returns true if OpenGL is used as rendering API.
    bool IsOpenGL() const;

    // Returns true if Vulkan is used as rendering API.
    bool IsVulkan() const;

    // Returns true if Direct3D is used as rendering API.
    bool IsDirect3D() const;

    // Returns true if Metal is used as rendering API.
    bool IsMetal() const;

    // Returns true if the screen origin of the selected renderer is lower-left. See RenderingCapabilities::screenOrigin.
    bool IsScreenOriginLowerLeft() const;

    // Returns true if the projection matrix is in a right-handed coordinate system.
    // By default, the ExampleBase puts its projection into a left-handed coordinate system.
    bool HasRightHandedProjection() const;

    // Returns the value of the Z-axis for this projection. +1 for left-handed, -1 for right-handed.
    float GetProjectionZAxis() const;

    // Returns a perspective projection with the specified parameters for the respective renderer.
    Gs::Matrix4f PerspectiveProjection(float aspectRatio, float near, float far, float fov) const;

    // Returns an orthogonal projection with the specified parameters for the respective renderer.
    Gs::Matrix4f OrthogonalProjection(float width, float height, float near, float far) const;

    // Returns a quoternion for the specified rotation
    Gs::Quaternionf Rotation(float pitch, float yaw) const;

    // Rotates the specified quaternion in a trackball motion (like in Blender).
    void TrackballRotation(Gs::Quaternionf& rotation, bool isStartPosition = false, const LLGL::Offset2D* cursorPosition = nullptr);

    // Returns true if the specified shading language is supported.
    bool Supported(const LLGL::ShadingLanguage shadingLanguage) const;

    // Quits the application by closing the window. This is used to quit prematurely when loading a PSO has failed.
    void Quit(int returnCode = 0);

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
    LLGL::Buffer* CreateVertexBuffer(const Container& vertices, std::uint32_t stride)
    {
        LLGL::BufferDescriptor bufferDesc = LLGL::VertexBufferDesc(GetArraySize(vertices), stride);
        bufferDesc.debugName = "VertexBuffer";
        return renderer->CreateBuffer(bufferDesc, &vertices[0]);
    }

    template <typename Container>
    LLGL::Buffer* CreateIndexBuffer(const Container& indices, const LLGL::Format format)
    {
        LLGL::BufferDescriptor bufferDesc = LLGL::IndexBufferDesc(GetArraySize(indices), format);
        bufferDesc.debugName = "IndexBuffer";
        return renderer->CreateBuffer(bufferDesc, &indices[0]);
    }

    template <typename T>
    LLGL::Buffer* CreateConstantBuffer(const T& initialData)
    {
        static_assert(!std::is_pointer<T>::value, "buffer type must not be a pointer");
        LLGL::BufferDescriptor bufferDesc = LLGL::ConstantBufferDesc(sizeof(T));
        bufferDesc.debugName = "ConstantBuffer";
        return renderer->CreateBuffer(bufferDesc, &initialData);
    }

};


#if defined LLGL_OS_ANDROID

#define LLGL_ANDROID_STDERR(...) \
    ((void)__android_log_print(ANDROID_LOG_ERROR, "threaded_app", __VA_ARGS__))

template <typename T>
void RunExample(android_app* state)
{
    #if LLGL_EXCEPTIONS_SUPPORTED
    try
    #endif
    {
        ExampleBase::SetAndroidApp(state);
        T tutorial;
        tutorial.Run();
    }
    #if LLGL_EXCEPTIONS_SUPPORTED
    catch (const std::exception& e)
    {
        LLGL_ANDROID_STDERR("%s\n", e.what());
    }
    #endif
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
    int returnCode = 0;

    #if LLGL_EXCEPTIONS_SUPPORTED
    try
    #endif
    {
        ExampleBase::ParseProgramArgs(argc, argv);
        T example;
        returnCode = example.Run();
    }
    #if LLGL_EXCEPTIONS_SUPPORTED
    catch (const std::exception& e)
    {
        LLGL::Log::Errorf("%s\n", e.what());
        returnCode = 1;
    }
    #endif

    #if _WIN32
    if (returnCode != 0) { system("pause"); }
    #endif

    return returnCode;
}

#define LLGL_IMPLEMENT_EXAMPLE(CLASS)           \
    int main(int argc, char* argv[])            \
    {                                           \
        return RunExample<CLASS>(argc, argv);   \
    }

#endif // /LLGL_OS_*


#endif

