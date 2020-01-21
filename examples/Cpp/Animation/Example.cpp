/*
 * Example.cpp (Example_Animation)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>


class Example_Animation : public ExampleBase
{

    LLGL::PipelineLayout*       pipelineLayout          = nullptr;
    LLGL::ResourceHeap*         resourceHeap            = nullptr;
    LLGL::ShaderProgram*        shaderProgram           = nullptr;
    LLGL::PipelineState*        pipelineScene           = nullptr;

    LLGL::Buffer*               vertexBuffer            = nullptr;
    LLGL::Buffer*               constantBuffer          = nullptr;

    LLGL::Texture*              colorMap                = nullptr;
    LLGL::Sampler*              linearSampler           = nullptr;

    TriangleMesh                meshStairsTop;
    TriangleMesh                meshStairsBottom;
    TriangleMesh                meshBall;

    const Gs::Vector2f          viewRotationOrigin      = { -33.4f, 45.0f };
    const float                 viewDistanceToCenter    = 15.0f;
    const float                 ballJumpHeight          = 0.5f;

    float                       viewRotationAnim        = 0.0f;
    Gs::Vector2f                viewRotation            = viewRotationOrigin;
    Gs::Vector2f                viewRotationPrev;

    struct Settings
    {
        Gs::Matrix4f            wMatrix;
        Gs::Matrix4f            vpMatrix;
        Gs::Vector3f            lightDir                = Gs::Vector3f(-0.25f, -0.7f, 1.25f).Normalized();
        float                   shininess               = 90.0f;                        // Blinn-phong specular power factor
        Gs::Vector3f            viewPos;                                                // World-space camera position
        float                   pad1;
        LLGL::ColorRGBAf        albedo                  = { 1.0f, 1.0f, 1.0f, 1.0f };   // Albedo material color
    }
    settings;

    struct Ball
    {
        Gs::Vector3f            position;
        Gs::Vector3f            scale;
        LLGL::ColorRGBf         color;
        std::size_t             frame               = 0u;
        float                   frameInterpolator   = 0.0f;
    };

    std::vector<Ball>           balls;

    const std::array<Gs::Vector2f, 15> gridPosFrames
    {{
        Gs::Vector2f{  0.0f, -3.0f },
        Gs::Vector2f{ +1.0f, -3.0f },
        Gs::Vector2f{ +2.0f, -3.0f },
        Gs::Vector2f{ +2.0f, -2.0f },
        Gs::Vector2f{ +2.0f, -1.0f },
        Gs::Vector2f{ +2.0f,  0.0f },
        Gs::Vector2f{ +2.0f, +1.0f },
        Gs::Vector2f{ +2.0f, +2.0f },
        Gs::Vector2f{ +1.0f, +2.0f },
        Gs::Vector2f{  0.0f, +2.0f },
        Gs::Vector2f{ -1.0f, +2.0f },
        Gs::Vector2f{ -2.0f, +2.0f },
        Gs::Vector2f{ -3.0f, +2.0f },
        Gs::Vector2f{ -3.0f, +1.0f },
        Gs::Vector2f{ -3.0f,  0.0f }
    }};

public:

    Example_Animation() :
        ExampleBase { L"LLGL Example: Animation" }
    {
        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        shaderProgram = LoadStandardShaderProgram({ vertexFormat });
        CreateTextures();
        CreateSamplers();
        CreatePipelineLayouts();
        CreatePipelines();
        CreateResourceHeaps();

        commands->SetClearColor(backgroundColor);

        // Add balls to scene
        AddBall(LLGL::ColorRGBf{ 1, 0, 0 });
        AddBall(LLGL::ColorRGBf{ 0, 1, 0 }, 5, 0.33f);
        AddBall(LLGL::ColorRGBf{ 0, 0, 1 }, 10, 0.66f);
    }

private:

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "normal",   LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float  });
        vertexFormat.SetStride(sizeof(TexturedVertex));

        // Load 3D models
        std::vector<TexturedVertex> vertices;
        meshStairsTop       = LoadObjModel(vertices, "../../Media/Models/PenroseStairs-Top.obj");
        meshStairsBottom    = LoadObjModel(vertices, "../../Media/Models/PenroseStairs-Bottom.obj");
        meshBall            = LoadObjModel(vertices, "../../Media/Models/IcoSphere.obj");

        meshStairsTop.color     = { 1.0f, 1.0f, 1.0f, 1.0f };
        meshStairsBottom.color  = { 1.0f, 1.0f, 0.0f, 0.0f };

        // Create vertex, index, and constant buffer
        vertexBuffer = CreateVertexBuffer(vertices, vertexFormat);
        constantBuffer = CreateConstantBuffer(settings);

        return vertexFormat;
    }

    void CreateTextures()
    {
        // Load color map texture
        colorMap = LoadTexture("../../Media/Textures/TilesGray512.jpg");
    }

    void CreateSamplers()
    {
        // Create default sampler state
        linearSampler = renderer->CreateSampler(LLGL::SamplerDescriptor{});
    }

    void CreatePipelineLayouts()
    {
        // Create pipeline layouts for shadow-map and scene rendering
        if (IsOpenGL())
        {
            pipelineLayout = renderer->CreatePipelineLayout(
                LLGL::PipelineLayoutDesc("cbuffer(Settings@1):frag:vert, texture(colorMap@2):frag, sampler(2):frag")
            );
        }
        else
        {
            pipelineLayout = renderer->CreatePipelineLayout(
                LLGL::PipelineLayoutDesc("cbuffer(Settings@1):frag:vert, texture(colorMap@2):frag, sampler(linearSampler@3):frag")
            );
        }
    }

    void CreatePipelines()
    {
        // Create graphics pipeline for scene rendering
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.shaderProgram                  = shaderProgram;
                pipelineDesc.renderPass                     = context->GetRenderPass();
                pipelineDesc.pipelineLayout                 = pipelineLayout;
                pipelineDesc.depth.testEnabled              = true;
                pipelineDesc.depth.writeEnabled             = true;
                pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;
                pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
            }
            pipelineScene = renderer->CreatePipelineState(pipelineDesc);
        }
    }

    void CreateResourceHeaps()
    {
        // Create resource heap for scene rendering
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout = pipelineLayout;
            resourceHeapDesc.resourceViews  = { constantBuffer, colorMap, linearSampler };
        }
        resourceHeap = renderer->CreateResourceHeap(resourceHeapDesc);
    }

    Gs::Vector3f GetGridPos(std::size_t frame) const
    {
        return Gs::Vector3f
        {
            gridPosFrames[frame].x + 0.5f,
            3.3f - static_cast<float>(frame) * 0.2f,
            gridPosFrames[frame].y + 0.5f
        };
    }

    void AddBall(const LLGL::ColorRGBf& color, std::size_t initialFrame = 0, float t = 0.0f)
    {
        Ball ball;
        {
            ball.position           = GetGridPos(initialFrame);
            ball.color              = color;
            ball.frame              = initialFrame;
            ball.frameInterpolator  = t;
        }
        balls.push_back(ball);
    }

    void UpdateBallAnimation(Ball& ball, float dt)
    {
        // Update frame interpolation
        ball.frameInterpolator += dt * 2.0f;

        while (ball.frameInterpolator > 1.0f)
        {
            ball.frameInterpolator -= 1.0f;
            ball.frame++;

            if (ball.frame + 1 >= gridPosFrames.size())
            {
                ball.position   = GetGridPos(0);
                ball.frame      = 0;
            }
        }

        float t     = ball.frameInterpolator;
        float ts    = Gs::SmoothStep(t);

        // Add scaling to animate bouncing
        float s = 0.0f;
        if (ts <= 0.1f)
            s = std::cos((ts * 5.0f) * Gs::pi);
        else if (ts >= 0.9f)
            s = std::sin(((ts - 0.9f) * 5.0f) * Gs::pi);

        // Interpolate transformation between current and next frame
        ball.position = Gs::Lerp(GetGridPos(ball.frame), GetGridPos(ball.frame + 1), t);
        ball.position.y += std::sin(t * Gs::pi) * ballJumpHeight;
        ball.scale = Gs::Vector3f{ 1.0f + s*0.1f, 1.0f - s*0.3f, 1.0f + s*0.1f };
    }

    void UpdateScene(float dt)
    {
        // Update animation
        if (input->KeyPressed(LLGL::Key::LButton))
        {
            auto motion = input->GetMouseMotion();
            viewRotation.x += static_cast<float>(motion.y) * 0.25f;
            viewRotation.x = Gs::Clamp(viewRotation.x, -90.0f, 90.0f);
            viewRotation.y += static_cast<float>(motion.x) * 0.25f;
        }
        else if (input->KeyUp(LLGL::Key::LButton))
        {
            // Reset animation
            viewRotationAnim    = 1.0f;
            viewRotationPrev    = viewRotation;
        }
        else if (viewRotationAnim > 0.0f)
        {
            viewRotationAnim    = viewRotationAnim - dt * 3.0f;
            viewRotation        = Gs::Lerp(viewRotationOrigin, viewRotationPrev, std::max(0.0f, viewRotationAnim*viewRotationAnim));
        }
        else
        {
            viewRotationAnim    = 0.0f;
            viewRotation        = viewRotationOrigin;
        }

        // Initialize camera matrices for orthogonal projection
        const float winSize = 8.0f;
        projection = OrthogonalProjection(winSize * GetAspectRatio(), winSize, 0.1f, 100.0f);

        // Update view transformation
        settings.vpMatrix.LoadIdentity();
        Gs::RotateFree(settings.vpMatrix, { 0, 1, 0 }, Gs::Deg2Rad(viewRotation.y));
        Gs::RotateFree(settings.vpMatrix, { 1, 0, 0 }, Gs::Deg2Rad(viewRotation.x));
        Gs::Translate(settings.vpMatrix, { 0, 0, -viewDistanceToCenter });
        settings.viewPos = Gs::TransformVector(settings.vpMatrix, Gs::Vector3f{ 0, 0, 0 });
        settings.vpMatrix.MakeInverse();
        settings.vpMatrix = projection * settings.vpMatrix;

        // Update ball animations
        for (auto& ball : balls)
            UpdateBallAnimation(ball, dt);
    }

    void RenderMesh(const TriangleMesh& mesh)
    {
        settings.wMatrix = mesh.transform;
        settings.albedo = mesh.color;
        commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));
        commands->Draw(mesh.numVertices, mesh.firstVertex);
    }

    void RenderBall(const Ball& ball)
    {
        // Set ball color
        settings.albedo = LLGL::ColorRGBAf{ ball.color.r, ball.color.g, ball.color.b, 0.0f };

        // Update model-to-world transformation matrix
        settings.wMatrix.LoadIdentity();
        Gs::Translate(settings.wMatrix, ball.position);
        Gs::Scale(settings.wMatrix, ball.scale*0.3f);

        // Submit data to constant buffer
        commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

        // Draw ball
        commands->Draw(meshBall.numVertices, meshBall.firstVertex);
    }

    void RenderScene()
    {
        commands->SetPipelineState(*pipelineScene);
        commands->SetResourceHeap(*resourceHeap);

        RenderMesh(meshStairsBottom);
        RenderMesh(meshStairsTop);

        for (const auto& ball : balls)
            RenderBall(ball);
    }

    void OnDrawFrame() override
    {
        // Update scene by user input
        timer->MeasureTime();
        UpdateScene(static_cast<float>(timer->GetDeltaTime()));

        commands->Begin();
        {
            // Bind common input assembly
            commands->SetVertexBuffer(*vertexBuffer);

            // Render everything directly into the render context
            commands->BeginRenderPass(*context);
            {
                commands->Clear(LLGL::ClearFlags::All);
                commands->SetViewport(context->GetResolution());
                RenderScene();
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_Animation);



