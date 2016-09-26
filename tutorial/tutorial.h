/*
 * tutorial.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_TUTORIAL_H__
#define __LLGL_TUTORIAL_H__


#include <LLGL/LLGL.h>
#include <Gauss/Gauss.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <type_traits>


class Tutorial
{

public:

    virtual ~Tutorial()
    {
    }

    void Run()
    {
        while (context->GetWindow().ProcessEvents() && !input->KeyDown(LLGL::Key::Escape))
        {
            profilerObj_->ResetCounters();
            OnDrawFrame();
        }
    }

private:

    class Debugger : public LLGL::RenderingDebugger
    {
        
        void OnError(LLGL::ErrorType type, Message& message) override
        {
            std::cerr << "ERROR: " << message.GetSource() << ": " << message.GetText() << std::endl;
            message.Block();
        }

        void OnWarning(LLGL::WarningType type, Message& message) override
        {
            std::cerr << "WARNING: " << message.GetSource() << ": " << message.GetText() << std::endl;
            message.Block();
        }

    };

    std::unique_ptr<LLGL::RenderingProfiler>    profilerObj_;
    std::unique_ptr<LLGL::RenderingDebugger>    debuggerObj_;

protected:

    std::shared_ptr<LLGL::RenderSystem>         renderer;
    LLGL::RenderContext*                        context     = nullptr;
    std::shared_ptr<LLGL::Input>                input;

    std::unique_ptr<LLGL::Timer>                timer;
    const LLGL::RenderingProfiler&              profiler;

    Gs::Matrix4f                                projection;

    virtual void OnDrawFrame() = 0;

    Tutorial(
        const std::string& rendererModule,
        const std::wstring& title,
        const LLGL::Size& resolution = { 800, 600 },
        unsigned int multiSampling = 8) :
            profilerObj_( new LLGL::RenderingProfiler() ),
            debuggerObj_( new Debugger()                ),
            timer       ( LLGL::Timer::Create()         ),
            profiler    ( *profilerObj_                 )
    {
        // Create render system
        renderer = LLGL::RenderSystem::Load(rendererModule, profilerObj_.get(), debuggerObj_.get());

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;
        {
            contextDesc.videoMode.resolution    = resolution;
            contextDesc.vsync.enabled           = true;
            contextDesc.antiAliasing.enabled    = (multiSampling > 1);
            contextDesc.antiAliasing.samples    = multiSampling;
        }
        context = renderer->CreateRenderContext(contextDesc);

        // Set window title
        auto rendererName = renderer->GetName();
        context->GetWindow().SetTitle(title + L" ( " + std::wstring(rendererName.begin(), rendererName.end()) + L" )");

        // Add input event listener to window
        input = std::make_shared<LLGL::Input>();
        context->GetWindow().AddEventListener(input);

        // Initialize default projection matrix
        projection = Gs::ProjectionMatrix4f::Perspective(GetAspectRatio(), 0.1f, 100.0f, Gs::Deg2Rad(45.0f)).ToMatrix4();

        // Set dark blue as default clear color
        context->SetClearColor({ 0.1f, 0.1f, 0.4f });
    }

    struct TutorialShaderDescriptor
    {
        TutorialShaderDescriptor(
            LLGL::ShaderType type, const std::string& filename) :
                type    ( type     ),
                filename( filename )
        {
        }

        TutorialShaderDescriptor(
            LLGL::ShaderType type, const std::string& filename, const std::string& entryPoint, const std::string& target) :
                type        ( type       ),
                filename    ( filename   ),
                entryPoint  ( entryPoint ),
                target      ( target     )
        {
        }

        LLGL::ShaderType    type;
        std::string         filename;
        std::string         entryPoint;
        std::string         target;
    };

    LLGL::ShaderProgram* LoadShaderProgram(
        const std::vector<TutorialShaderDescriptor>& shaderDescs,
        const std::vector<LLGL::VertexAttribute>& vertexAttribs = {})
    {
        // Create shader program
        LLGL::ShaderProgram* shaderProgram = renderer->CreateShaderProgram();

        for (const auto& desc : shaderDescs)
        {
            // Read shader file
            std::ifstream file(desc.filename);

            if (!file.good())
                throw std::runtime_error("failed to open file: \"" + desc.filename + "\"");

            std::string shaderCode(
                ( std::istreambuf_iterator<char>(file) ),
                ( std::istreambuf_iterator<char>() )
            );

            // Create shader and compile shader
            auto shader = renderer->CreateShader(desc.type);
            shader->Compile(LLGL::ShaderSource(shaderCode, desc.entryPoint, desc.target, LLGL::ShaderCompileFlags::Debug));

            // Print info log (warnings and errors)
            std::string log = shader->QueryInfoLog();
            if (!log.empty())
                std::cerr << log << std::endl;

            // Attach vertex- and fragment shader to the shader program
            shaderProgram->AttachShader(*shader);
        }

        // Bind vertex attribute layout (this is not required for a compute shader program)
        if (!vertexAttribs.empty())
            shaderProgram->BindVertexAttributes(vertexAttribs);

        // Link shader program and check for errors
        if (!shaderProgram->LinkShaders())
            throw std::runtime_error(shaderProgram->QueryInfoLog());

        return shaderProgram;
    }

    LLGL::ShaderProgram* LoadStandardShaderProgram(const LLGL::VertexFormat& vertexFormat)
    {
        // Load shader program
        if (renderer->QueryRenderingCaps().hasHLSL)
        {
            return LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "shader.hlsl", "VS", "vs_5_0" },
                    { LLGL::ShaderType::Fragment, "shader.hlsl", "PS", "ps_5_0" }
                },
                vertexFormat.GetAttributes()
            );
        }
        else
        {
            return LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "vertex.glsl" },
                    { LLGL::ShaderType::Fragment, "fragment.glsl" }
                },
                vertexFormat.GetAttributes()
            );
        }
    }

    // Generates eight vertices for a unit cube.
    std::vector<Gs::Vector3f> GenerateCubeVertices()
    {
        return
        {
            { -1, -1, -1 }, { -1,  1, -1 }, {  1,  1, -1 }, {  1, -1, -1 },
            { -1, -1,  1 }, { -1,  1,  1 }, {  1,  1,  1 }, {  1, -1,  1 },
        };
    }

    // Generates 36 indices for a unit cube of eight vertices
    // (36 = 3 indices per triangle * 2 triangles per cube face * 6 faces).
    std::vector<std::uint32_t> GenerateCubeTriangelIndices()
    {
        return
        {
            0, 1, 2, 0, 2, 3, // front
            3, 2, 6, 3, 6, 7, // right
            4, 5, 1, 4, 1, 0, // left
            1, 5, 6, 1, 6, 2, // top
            4, 0, 3, 4, 3, 7, // bottom
            7, 6, 5, 7, 5, 4, // back
        };
    }

    // Generates 24 indices for a unit cube of eight vertices.
    // (24 = 4 indices per quad * 1 quad per cube face * 6 faces)
    std::vector<std::uint32_t> GenerateCubeQuadlIndices()
    {
        return
        {
            0, 1, 3, 2, // front
            3, 2, 7, 6, // right
            4, 5, 0, 1, // left
            1, 5, 2, 6, // top
            4, 0, 7, 3, // bottom
            7, 6, 4, 5, // back
        };
    }

    template <typename VertexType>
    LLGL::VertexBuffer* CreateVertexBuffer(const std::vector<VertexType>& vertices, const LLGL::VertexFormat& vertexFormat)
    {
        auto vertexBuffer = renderer->CreateVertexBuffer();
        renderer->SetupVertexBuffer(*vertexBuffer, vertices.data(), vertices.size() * sizeof(VertexType), LLGL::BufferUsage::Static, vertexFormat);
        return vertexBuffer;
    }

    template <typename IndexType>
    LLGL::IndexBuffer* CreateIndexBuffer(const std::vector<IndexType>& indices, const LLGL::IndexFormat& indexFormat)
    {
        auto indexBuffer = renderer->CreateIndexBuffer();
        renderer->SetupIndexBuffer(*indexBuffer, indices.data(), indices.size() * sizeof(IndexType), LLGL::BufferUsage::Static, indexFormat);
        return indexBuffer;
    }

    template <typename Buffer>
    LLGL::ConstantBuffer* CreateConstantBuffer(const Buffer& buffer)
    {
        static_assert(!std::is_pointer<Buffer>::value, "buffer type must not be a pointer");
        auto constantBuffer = renderer->CreateConstantBuffer();
        renderer->SetupConstantBuffer(*constantBuffer, &buffer, sizeof(buffer), LLGL::BufferUsage::Dynamic);
        return constantBuffer;
    }

    template <typename Buffer>
    void UpdateConstantBuffer(LLGL::ConstantBuffer* constantBuffer, const Buffer& buffer)
    {
        GS_ASSERT(constantBuffer != nullptr);
        renderer->WriteConstantBuffer(*constantBuffer, &buffer, sizeof(buffer), 0);
    }

    // Returns the aspect ratio of the render context resolution (X:Y).
    float GetAspectRatio() const
    {
        auto resolution = context->GetVideoMode().resolution.Cast<float>();
        return (resolution.x / resolution.y);
    }

};


template <typename T>
int RunTutorial()
{
    try
    {
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

#define LLGL_IMPLEMENT_TUTORIAL(CLASS)  \
    int main()                          \
    {                                   \
        return RunTutorial<CLASS>();    \
    }


#endif

