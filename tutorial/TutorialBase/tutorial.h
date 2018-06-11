/*
 * tutorial.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TUTORIAL_H
#define LLGL_TUTORIAL_H


#include <LLGL/LLGL.h>
#include <LLGL/Utility.h>
#include <Gauss/Gauss.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <type_traits>
#include "geometry.h"


/*
 * Global helper functions
 */

// Let the user choose a renderer module (using std::cin).
std::string GetSelectedRendererModule(int argc, char* argv[]);

// Read a text file to a string.
std::string ReadFileContent(const std::string& filename);

// Read a binary file to a buffer.
std::vector<char> ReadFileBuffer(const std::string& filename);

// Load image from file, create texture, upload image into texture, and generate MIP-maps.
LLGL::Texture* LoadTextureWithRenderer(LLGL::RenderSystem& renderSys, const std::string& filename);

// Save texture image to a PNG file.
bool SaveTextureWithRenderer(LLGL::RenderSystem& renderSys, LLGL::Texture& texture, const std::string& filename, std::uint32_t mipLevel = 0);


/*
 * Tutorial base class
 */

class Tutorial
{

public:

    static void SelectRendererModule(int argc, char* argv[]);

    virtual ~Tutorial() = default;

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
            const std::string&  target
        );

        LLGL::ShaderType    type;
        std::string         filename;
        std::string         entryPoint;
        std::string         target;
    };

private:

    class ResizeEventHandler : public LLGL::Window::EventListener
    {

        public:

            ResizeEventHandler(Tutorial& tutorial, LLGL::RenderContext* context, Gs::Matrix4f& projection);

            void OnResize(LLGL::Window& sender, const LLGL::Extent2D& clientAreaSize) override;
            void OnTimer(LLGL::Window& sender, std::uint32_t timerID);

        private:

            Tutorial&               tutorial_;
            LLGL::RenderContext*    context_;
            Gs::Matrix4f&           projection_;

    };

    struct ShaderProgramRecall
    {
        std::vector<TutorialShaderDescriptor>   shaderDescs;
        std::vector<LLGL::Shader*>              shaders;
        std::vector<LLGL::VertexFormat>         vertexFormats;
        LLGL::StreamOutputFormat                streamOutputFormat;
    };

    std::unique_ptr<LLGL::RenderingProfiler>    profilerObj_;
    std::unique_ptr<LLGL::RenderingDebugger>    debuggerObj_;

    std::map< LLGL::ShaderProgram*,
              ShaderProgramRecall >             shaderPrograms_;

    bool                                        loadingDone_    = false;

    static std::string                          rendererModule_;

protected:

    friend class ResizeEventHandler;

    const LLGL::ColorRGBAf                      defaultClearColor { 0.1f, 0.1f, 0.4f };

    // Render system
    std::unique_ptr<LLGL::RenderSystem>         renderer;

    // Main render context
    LLGL::RenderContext*                        context         = nullptr;

    // Main command buffer
    union
    {
        LLGL::CommandBuffer*                    commands        = nullptr;
        LLGL::CommandBufferExt*                 commandsExt;
    };

    // Command queue
    LLGL::CommandQueue*                         commandQueue    = nullptr;

    std::shared_ptr<LLGL::Input>                input;

    std::unique_ptr<LLGL::Timer>                timer;
    const LLGL::RenderingProfiler&              profiler;

    Gs::Matrix4f                                projection;

    Tutorial(
        const std::wstring&     title,
        const LLGL::Extent2D&   resolution  = { 800, 600 },
        std::uint32_t           samples     = 8,
        bool                    vsync       = true,
        bool                    debugger    = true
    );

    virtual void OnDrawFrame() = 0;

    // Creats a shader program and loads all specified shaders from file.
    LLGL::ShaderProgram* LoadShaderProgram(
        const std::vector<TutorialShaderDescriptor>&    shaderDescs,
        const std::vector<LLGL::VertexFormat>&          vertexFormats       = {},
        const LLGL::StreamOutputFormat&                 streamOutputFormat  = {}
    );

    // Reloads the specified shader program from the previously specified shader source files.
    bool ReloadShaderProgram(LLGL::ShaderProgram* shaderProgram);

    // Load standard shader program (with vertex- and fragment shaders).
    LLGL::ShaderProgram* LoadStandardShaderProgram(const std::vector<LLGL::VertexFormat>& vertexFormats);

protected:

    // Load image from file, create texture, upload image into texture, and generate MIP-maps.
    LLGL::Texture* LoadTexture(const std::string& filename);

    // Save texture image to a PNG file.
    bool SaveTexture(LLGL::Texture& texture, const std::string& filename, std::uint32_t mipLevel = 0);

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
    LLGL::Buffer* CreateIndexBuffer(const std::vector<IndexType>& indices, const LLGL::IndexFormat& indexFormat)
    {
        return renderer->CreateBuffer(
            LLGL::IndexBufferDesc(static_cast<std::uint32_t>(indices.size() * sizeof(IndexType)), indexFormat),
            indices.data()
        );
    }

    template <typename Buffer>
    LLGL::Buffer* CreateConstantBuffer(const Buffer& buffer)
    {
        static_assert(!std::is_pointer<Buffer>::value, "buffer type must not be a pointer");
        return renderer->CreateBuffer(
            LLGL::ConstantBufferDesc(sizeof(buffer)),
            &buffer
        );
    }

    template <typename T>
    void UpdateBuffer(LLGL::Buffer* buffer, const T& data)
    {
        GS_ASSERT(buffer != nullptr);
        renderer->WriteBuffer(*buffer, &data, sizeof(data), 0);
    }

    // Returns the aspect ratio of the render context resolution (X:Y).
    float GetAspectRatio() const;

    // Returns ture if OpenGL is used as rendering API.
    bool IsOpenGL() const;

    // Used by the window resize handler
    bool IsLoadingDone() const;

    // Returns a projection with the specified parameters for the respective renderer.
    Gs::Matrix4f PerspectiveProjection(float aspectRatio, float near, float far, float fov);

    // Returns true if the specified shading language is supported.
    bool Supported(const LLGL::ShadingLanguage shadingLanguage) const;

};


template <typename T>
int RunTutorial(int argc, char* argv[])
{
    try
    {
        /* Run tutorial */
        Tutorial::SelectRendererModule(argc, argv);
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

#define LLGL_IMPLEMENT_TUTORIAL(CLASS)          \
    int main(int argc, char* argv[])            \
    {                                           \
        return RunTutorial<CLASS>(argc, argv);  \
    }


#endif

