/*
 * MorphTargets.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <functional>



static int ModuloSignInt(int a, int b)
{
    return (((a % b + b)) % b);
}

// Helper function to calculate modulo operator with signed values to wrap around start and end keyframes.
static std::uint32_t ModuloSubtract(std::uint32_t lhs, std::uint32_t rhs, std::uint32_t wrap)
{
    int a = static_cast<int>(lhs) - static_cast<int>(rhs);
    int b = static_cast<int>(wrap);
    return static_cast<std::uint32_t>(ModuloSignInt(a, b));
}

// This example renders an open book an animates the pages via morph-target animation,
// a set of keyframe meshes that are interpolated in the vertex shader.
class Example_MorphTargets : public ExampleBase
{

    static constexpr float      pageTurningTime         = 2.0f;     // Time to turn a page in seconds
    static constexpr int        numBookPageKeyframes    = 16;       // Number of keyframe meshes to load from 'BookPageKeyframe_<N>.obj' pattern

    // Function signature when a page is finished animating.
    using AnimFinishCallback = std::function<void(bool forwards)>;

    enum MorphTargetVertexBuffer
    {
        MTVB_CurrentTarget = 0,
        MTVB_NextTarget,
        MTVB_TexCoords,

        MTVB_Count,
    };

    enum PageFace
    {
        PageFace_Front = 0,
        PageFace_Back,

        PageFace_Count,
    };

    enum BindingTable
    {
        // Resources:
        BindingTable_SceneView = 0,
        BindingTable_ColorMap,

        // Uniforms:
        BindingTable_AnimationState = 0,
        BindingTable_InvertXAxis,
    };

    struct StaticMesh
    {
        std::uint32_t   numVertices = 0;
        std::uint32_t   firstVertex = 0;
        LLGL::Texture*  colorMap    = nullptr;
    };

    // Morph-target meshes are composed of three vertex buffers:
    // (1) texture-coordinates, (2) position & normal target[0], (3) position & normal target[1]
    struct MorphTargetKeyframeVertex
    {
        Gs::Vector3f position;
        Gs::Vector3f normal;
    };

    struct MorphTargetMesh
    {
        std::uint32_t               numVertices             = 0;
        std::uint64_t               texCoordsVbufferOffset  = 0;
        std::vector<std::uint64_t>  keyframeVbufferOffset;
    };

    // Tracks animation state and the page to render in the 3D book model.
    struct MorphTargetAnimation
    {
        std::uint32_t       numKeyframes        = 0;
        std::uint32_t       currentKeyframe     = 0;
        std::uint32_t       nextKeyframe        = 0;
        float               interpolationFactor = 0.0f;
        float               animationSpeed      = 0.0f;
        bool                isReverse           = false;
        LLGL::Texture*      faceTextures[2]     = {};
        AnimFinishCallback  finishCallback;

        void Play(std::uint32_t keyframes, float speed = 1.0f, bool reverse = false, LLGL::Texture* frontTex = nullptr, LLGL::Texture* backTex = nullptr, const AnimFinishCallback& callback = nullptr)
        {
            if (keyframes >= 2)
            {
                numKeyframes        = keyframes;
                interpolationFactor = 0.0f;
                animationSpeed      = speed;
                isReverse           = reverse;
                if (speed > 0.0f)
                {
                    currentKeyframe = 0;
                    nextKeyframe    = 1;
                }
                else
                {
                    currentKeyframe = numKeyframes - 1;
                    nextKeyframe    = currentKeyframe - 1;
                }
                faceTextures[1]     = frontTex;
                faceTextures[0]     = backTex;
                finishCallback      = callback;
            }
        }

        void Stop()
        {
            animationSpeed  = 0.0f;
            nextKeyframe    = currentKeyframe;
        }

        bool IsPlaying() const
        {
            return (nextKeyframe != currentKeyframe);
        }

        bool IsMovingForward() const
        {
            return (animationSpeed > 0.0f) != isReverse;
        }

        void Swap()
        {
            if (IsPlaying())
            {
                animationSpeed = -animationSpeed;
                isReverse = !isReverse;
                std::swap(currentKeyframe, nextKeyframe);
            }
        }

        void OnFinished()
        {
            if (finishCallback)
                finishCallback(IsMovingForward());
        }

        void Animate(float dt)
        {
            if (IsPlaying())
            {
                float jumpFrames = 0.0f;
                dt *= animationSpeed;
                if (dt > 0.0f)
                {
                    interpolationFactor = std::modf(interpolationFactor + dt, &jumpFrames);
                    const std::uint32_t jumpFramesCount = static_cast<std::uint32_t>(jumpFrames);
                    if (jumpFramesCount > 0)
                    {
                        if (currentKeyframe + jumpFramesCount + 1 < numKeyframes)
                        {
                            currentKeyframe += jumpFramesCount;
                            nextKeyframe    = currentKeyframe + 1;
                        }
                        else
                        {
                            OnFinished();
                            Stop();
                        }
                    }
                }
                else if (dt < 0.0f)
                {
                    interpolationFactor = std::modf(interpolationFactor - dt, &jumpFrames);
                    const std::uint32_t jumpFramesCount = static_cast<std::uint32_t>(jumpFrames);
                    if (jumpFramesCount > 0)
                    {
                        if (currentKeyframe > jumpFramesCount)
                        {
                            currentKeyframe -= jumpFramesCount;
                            nextKeyframe    = currentKeyframe - 1;
                        }
                        else
                        {
                            OnFinished();
                            Stop();
                        }
                    }
                }
            }
        }
    };

    std::vector<MorphTargetAnimation> animations; // List of all active animations

    struct alignas(16) SceneView
    {
        Gs::Matrix4f        wvpMatrix;
        Gs::Matrix4f        wMatrix;
        Gs::Vector3f        lightVec    = Gs::Vector3f(-0.25f, -1.0f, 0.5f).Normalized();
        LLGL::ColorRGBAf    baseColor   = { 1.0f, 1.0f, 1.0f, 1.0f };
    }
    sceneView;

    struct Scene
    {
        StaticMesh      meshBookShell;
        StaticMesh      meshLRestingPage;
        StaticMesh      meshRRestingPage;
        MorphTargetMesh meshMovingPage;
        int             leftPageNo          = 0;
    }
    scene;

    LLGL::Shader*               vsStaticMesh                        = nullptr;
    LLGL::Shader*               vsMorphTargetMesh                   = nullptr;
    LLGL::Shader*               fsBlinnPhong                        = nullptr;

    LLGL::PipelineLayout*       psoLayoutStatic                     = nullptr;
    LLGL::PipelineLayout*       psoLayoutMorphTarget                = nullptr;

    LLGL::PipelineState*        psoStaticMesh                       = nullptr;
    LLGL::PipelineState*        psoMorphTargetMesh[PageFace_Count]  = {};

    LLGL::Buffer*               meshBuffer                          = nullptr;  // Single mesh buffer containing vertex data for the entire scene
    LLGL::Buffer*               sceneViewCbuffer                    = nullptr;  // Scene view constant buffer

    LLGL::Texture*              bookShellTexture                    = nullptr;
    std::vector<LLGL::Texture*> pageTextures;                       // List of textures for all book pages to render

public:

    Example_MorphTargets() :
        ExampleBase { "LLGL Example: MorphTargets" }
    {
        // Create all graphics objects
        LoadTextures();
        LoadMeshes();
        LoadShaders();
        CreateStaticMeshPSO();
        CreateMorphTargetMeshPSO();

        // Update vectors for projection
        sceneView.lightVec.z *= GetProjectionZAxis();
    }

private:

    void LoadPageTextures(const std::initializer_list<const char*>& filenames)
    {
        for (const char* filename : filenames)
            pageTextures.push_back(LoadTexture(filename));
    }

    void LoadTextures()
    {
        bookShellTexture = LoadTexture("Book.png");

        LoadPageTextures(
            {
                "Logo_LLGL.png",
                "Logo_Direct3D12.png",
                "Logo_Direct3D11.png",
                "Logo_Vulkan.png",
                "Logo_OpenGL.png",
                "Logo_Metal.png",
            }
        );
    }

    StaticMesh LoadStaticMesh(std::vector<TexturedVertex>& outVertices, const std::string& filename, LLGL::Texture* colorMap)
    {
        TriangleMesh intermediateMesh = Load3DModel(outVertices, filename, 3, MeshFlags_FlipTexCoordV);

        StaticMesh outMesh;
        {
            outMesh.numVertices = intermediateMesh.numVertices;
            outMesh.firstVertex = intermediateMesh.firstVertex;
            outMesh.colorMap    = colorMap;
        }
        return outMesh;
    }

    template <typename TContainer>
    static std::size_t ByteSize(const TContainer& cont)
    {
        return sizeof(typename TContainer::value_type) * cont.size();
    }

    static std::string KeyframeMeshFilename(int keyframe)
    {
        return ("BookPageKeyframe_" + std::to_string(keyframe) + ".obj");
    }

    void LoadMeshes()
    {
        LLGL_VERIFY(numBookPageKeyframes >= 2 && "Need at least 2 keyframes to load");
        LLGL_VERIFY(bookShellTexture != nullptr);
        LLGL_VERIFY(pageTextures.size() >= 2);
        LLGL_VERIFY(pageTextures[0] != nullptr);
        LLGL_VERIFY(pageTextures[1] != nullptr);

        // Load static meshes
        std::vector<TexturedVertex> staticVertices;
        scene.meshBookShell     = LoadStaticMesh(staticVertices, "BookShell.obj", bookShellTexture);
        scene.meshLRestingPage  = LoadStaticMesh(staticVertices, "BookLeftPage.obj", pageTextures[0]);
        scene.meshRRestingPage  = LoadStaticMesh(staticVertices, "BookRightPage.obj", pageTextures[1]);

        // Load morph-target keyframes
        std::uint64_t numTotalKeyframeVertices = 0;
        std::vector<std::vector<MorphTargetKeyframeVertex>> keyframeMeshes;
        std::vector<Gs::Vector2f> morphTargetTexCoords;

        std::vector<TexturedVertex> intermediateVertices;

        keyframeMeshes.resize(numBookPageKeyframes);
        for (int keyframe = 0; keyframe < numBookPageKeyframes; ++keyframe)
        {
            // Load keyframe as static 3D model
            intermediateVertices = Load3DModel(KeyframeMeshFilename(keyframe), 4, MeshFlags_FlipTexCoordU | MeshFlags_FlipTexCoordV | MeshFlags_Triangulate);
            const std::size_t numKeyframeVertices = intermediateVertices.size();

            // Split static mesh into keyframe containers
            std::vector<MorphTargetKeyframeVertex>& keyframeVertices = keyframeMeshes[keyframe];
            keyframeVertices.resize(numKeyframeVertices);
            for (std::size_t vertexIndex = 0; vertexIndex < numKeyframeVertices; ++vertexIndex)
            {
                keyframeVertices[vertexIndex].position  = intermediateVertices[vertexIndex].position;
                keyframeVertices[vertexIndex].normal    = intermediateVertices[vertexIndex].normal;
            }

            // Read texture-coordinates only from the first keyframe.
            // Discard texture-coordinates from the other models as the same coordinates are shared across all keyframes.
            if (keyframe == 0)
            {
                morphTargetTexCoords.resize(numKeyframeVertices);
                for (std::size_t vertexIndex = 0; vertexIndex < numKeyframeVertices; ++vertexIndex)
                    morphTargetTexCoords[vertexIndex] = intermediateVertices[vertexIndex].texCoord;
            }

            // Keep track how keyframe vertices
            numTotalKeyframeVertices += numKeyframeVertices;
        }

        // Ensure all keyframes have the same number of vertices
        for (int keyframe = 1; keyframe < numBookPageKeyframes; ++keyframe)
        {
            if (keyframeMeshes[0].size() != keyframeMeshes[keyframe].size())
            {
                LLGL::Log::Errorf(
                    LLGL::Log::ColorFlags::StdError,
                    "Mismatch in number of vertices between keyframe '%s' (%uz) and keyframe '%s' (%uz)",
                    KeyframeMeshFilename(0       ).c_str(), keyframeMeshes[0       ].size(),
                    KeyframeMeshFilename(keyframe).c_str(), keyframeMeshes[keyframe].size()
                );
                Quit(1);
            }
        }

        // Create mesh buffer
        LLGL::BufferDescriptor meshBufferDesc;
        {
            meshBufferDesc.debugName    = "MeshBuffer(Static+MorphTargets)";
            meshBufferDesc.size         = ByteSize(staticVertices) + ByteSize(morphTargetTexCoords) + sizeof(MorphTargetKeyframeVertex) * numTotalKeyframeVertices;
            meshBufferDesc.stride       = sizeof(TexturedVertex); // Use static vertex stride as default; Change it 'on the fly' for morph-targets.
            meshBufferDesc.bindFlags    = LLGL::BindFlags::VertexBuffer;
        }
        meshBuffer = renderer->CreateBuffer(meshBufferDesc);

        // Fill mesh buffer with vertex data and store byte offests for sections
        std::uint64_t meshBufferWritePosition = 0;

        auto AppendVertexData = [this, &meshBufferWritePosition, &meshBufferDesc](const void* data, std::size_t dataSize) -> std::uint64_t
        {
            LLGL_VERIFY(meshBufferWritePosition + dataSize <= meshBufferDesc.size);
            const std::uint64_t startOffset = meshBufferWritePosition;
            renderer->WriteBuffer(*meshBuffer, startOffset, data, dataSize);
            meshBufferWritePosition += dataSize;
            return startOffset;
        };

        AppendVertexData(staticVertices.data(), ByteSize(staticVertices));

        // Fill mesh buffer with morph-target keyframes
        const std::uint32_t numKeyframes = static_cast<std::uint32_t>(keyframeMeshes.size());

        scene.meshMovingPage.numVertices = static_cast<std::uint32_t>(keyframeMeshes[0].size());
        scene.meshMovingPage.texCoordsVbufferOffset = AppendVertexData(morphTargetTexCoords.data(), ByteSize(morphTargetTexCoords));
        scene.meshMovingPage.keyframeVbufferOffset.resize(numKeyframes);

        for (std::uint32_t keyframe = 0; keyframe < numKeyframes; ++keyframe)
        {
            const std::vector<MorphTargetKeyframeVertex>& keyframeVertices = keyframeMeshes[keyframe];
            scene.meshMovingPage.keyframeVbufferOffset[keyframe] = AppendVertexData(keyframeVertices.data(), ByteSize(keyframeVertices));
        }

        // Create scene constant buffer
        sceneViewCbuffer = CreateConstantBuffer(sceneView, "SceneView.Cbuffer");
    }

    void LoadShaders()
    {
        // Load shader programs
        vsStaticMesh        = LoadVertexShader  ("MorphTargets", "VStaticMesh");
        vsMorphTargetMesh   = LoadVertexShader  ("MorphTargets", "VMorphTargetMesh");
        fsBlinnPhong        = LoadFragmentShader("MorphTargets", "PBlinnPhong");
    }

    void CreateStaticMeshPSO()
    {
        // Create PSO layout
        psoLayoutStatic = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneView@3):vert:frag,"
                "texture(colorMap@4):frag,"
              //"texture(alphaMask@5):frag,"
                "sampler(colorMapSampler@6){}:frag," // Static sampler
              //"sampler(alphaMaskSampler@7):frag,"

                "sampler<colorMap, colorMapSampler>(s_colorMapcolorMapSampler@4),"
              //"sampler<alphaMask, alphaMaskSampler>(s_alphaMaskalphaMaskSampler@6),"
            )
        );

        // Specify vertex format
        const LLGL::VertexAttribute staticMeshVertexAttribs[] =
        {
            LLGL::VertexAttribute{ "position", LLGL::Format::RGB32Float, 0, offsetof(TexturedVertex, position), sizeof(TexturedVertex) },
            LLGL::VertexAttribute{ "normal",   LLGL::Format::RGB32Float, 1, offsetof(TexturedVertex, normal  ), sizeof(TexturedVertex) },
            LLGL::VertexAttribute{ "texCoord", LLGL::Format::RG32Float,  2, offsetof(TexturedVertex, texCoord), sizeof(TexturedVertex) },
        };

        // Create graphics pipeline for static meshes
        LLGL::GraphicsPipelineDescriptor psoStaticMeshDesc;
        {
            psoStaticMeshDesc.inputVertexAttribs                = staticMeshVertexAttribs;
            psoStaticMeshDesc.vertexShader                      = vsStaticMesh;
            psoStaticMeshDesc.fragmentShader                    = fsBlinnPhong;
            psoStaticMeshDesc.renderPass                        = swapChain->GetRenderPass();
            psoStaticMeshDesc.pipelineLayout                    = psoLayoutStatic;
            psoStaticMeshDesc.depth.testEnabled                 = true;
            psoStaticMeshDesc.depth.writeEnabled                = true;
            psoStaticMeshDesc.rasterizer.cullMode               = LLGL::CullMode::Back; // Back-face culling for static meshes
            psoStaticMeshDesc.rasterizer.multiSampleEnabled     = (GetSampleCount() > 1);
        }
        psoStaticMesh = renderer->CreatePipelineState(psoStaticMeshDesc);
        ReportPSOErrors(psoStaticMesh);
    }

    void CreateMorphTargetMeshPSO()
    {
        // Create PSO layout
        psoLayoutMorphTarget = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneView@3):vert:frag,"
                "texture(colorMap@4):frag,"
                //"texture(alphaMask@5):frag,"
                "sampler(colorMapSampler@6){}:frag," // Static sampler
                //"sampler(alphaMaskSampler@7):frag,"

                "float(animationState.interpolationFactor)," // Interpolation factor as uniform to efficiently animate many morph targets
                "float(animationState.invertXAxis),"

                "sampler<colorMap, colorMapSampler>(s_colorMapcolorMapSampler@4),"
                //"sampler<alphaMask, alphaMaskSampler>(s_alphaMaskalphaMaskSampler@6),"
            )
        );

        // Specify vertex format with three vertex buffer slots:
        // (1) current keyframe
        // (2) next keyframe
        // (3) texture-coordinates shared between all keyframes (they don't change)
        const LLGL::VertexAttribute morphTargetMeshVertexAttribs[] =
        {
            // MTVB_CurrentTarget
            LLGL::VertexAttribute{ "positionA", LLGL::Format::RGB32Float, 0, offsetof(MorphTargetKeyframeVertex, position), sizeof(MorphTargetKeyframeVertex), MTVB_CurrentTarget },
            LLGL::VertexAttribute{ "normalA",   LLGL::Format::RGB32Float, 1, offsetof(MorphTargetKeyframeVertex, normal  ), sizeof(MorphTargetKeyframeVertex), MTVB_CurrentTarget },

            // MTVB_NextTarget
            LLGL::VertexAttribute{ "positionB", LLGL::Format::RGB32Float, 2, offsetof(MorphTargetKeyframeVertex, position), sizeof(MorphTargetKeyframeVertex), MTVB_NextTarget    },
            LLGL::VertexAttribute{ "normalB",   LLGL::Format::RGB32Float, 3, offsetof(MorphTargetKeyframeVertex, normal  ), sizeof(MorphTargetKeyframeVertex), MTVB_NextTarget    },

            // MTVB_TexCoords
            LLGL::VertexAttribute{ "texCoord",  LLGL::Format::RG32Float,  4, 0,                                             sizeof(Gs::Vector2f),              MTVB_TexCoords     },
        };

        // Create graphics pipeline for front facing animated pages
        LLGL::GraphicsPipelineDescriptor psoMorphTargetDesc;
        {
            psoMorphTargetDesc.inputVertexAttribs               = morphTargetMeshVertexAttribs;
            psoMorphTargetDesc.vertexShader                     = vsMorphTargetMesh;
            psoMorphTargetDesc.fragmentShader                   = fsBlinnPhong;
            psoMorphTargetDesc.renderPass                       = swapChain->GetRenderPass();
            psoMorphTargetDesc.pipelineLayout                   = psoLayoutMorphTarget;
            psoMorphTargetDesc.depth.testEnabled                = true;
            psoMorphTargetDesc.depth.writeEnabled               = true;
            psoMorphTargetDesc.rasterizer.cullMode              = LLGL::CullMode::Back;
            psoMorphTargetDesc.rasterizer.multiSampleEnabled    = (GetSampleCount() > 1);
        }
        psoMorphTargetMesh[PageFace_Front] = renderer->CreatePipelineState(psoMorphTargetDesc);
        ReportPSOErrors(psoMorphTargetMesh[PageFace_Front]);

        // Create graphics pipeline for back facing animated pages
        {
            psoMorphTargetDesc.rasterizer.cullMode              = LLGL::CullMode::Front;
        }
        psoMorphTargetMesh[PageFace_Back] = renderer->CreatePipelineState(psoMorphTargetDesc);
        ReportPSOErrors(psoMorphTargetMesh[PageFace_Back]);
    }

    LLGL::Texture* GetPageTexture(int pageNo)
    {
        const int numPages = static_cast<int>(pageTextures.size());
        const int pageTextureIndex = ModuloSignInt(pageNo, numPages);
        return pageTextures[pageTextureIndex];
    }

    void AnimatePage(int oldPage, int newPage0, int newPage1, float animSpeed = 1.0f, bool reverse = false)
    {
        MorphTargetAnimation anim;
        const std::uint32_t numKeyframes = static_cast<std::uint32_t>(scene.meshMovingPage.keyframeVbufferOffset.size());
        const float invTimePerKeyframe = static_cast<float>(numKeyframes) / pageTurningTime;

        LLGL::Texture* oldPageTex = GetPageTexture(oldPage);
        LLGL::Texture* newPage0Tex = GetPageTexture(newPage0);
        LLGL::Texture* newPage1Tex = GetPageTexture(newPage1);

        anim.Play(
            numKeyframes,
            animSpeed * invTimePerKeyframe,
            reverse,
            oldPageTex,
            newPage0Tex,
            [this, newPage0Tex, oldPageTex](bool forwards)
            {
                if (forwards)
                    this->scene.meshLRestingPage.colorMap = newPage0Tex;
                else
                    this->scene.meshRRestingPage.colorMap = oldPageTex;
            }
        );

        if (anim.IsMovingForward())
            this->scene.meshRRestingPage.colorMap = newPage1Tex;
        else
            this->scene.meshLRestingPage.colorMap = newPage1Tex;

        animations.push_back(anim);
    }

    bool ReverseLastPage(bool forwards)
    {
        for (auto it = animations.rbegin(); it != animations.rend(); ++it)
        {
            MorphTargetAnimation& anim = *it;
            if (anim.isReverse == forwards)
            {
                anim.Swap();
                return true;
            }
        }
        return false;
    }

    void UpdateUserInput()
    {
        const int numPages = static_cast<int>(pageTextures.size());

        if (input.KeyDown(LLGL::Key::Left) && !ReverseLastPage(true))
        {
            int oldPage     = scene.leftPageNo + 1;
            int newPage0    = ModuloSignInt(scene.leftPageNo + 2, numPages);
            int newPage1    = ModuloSignInt(scene.leftPageNo + 3, numPages);

            AnimatePage(oldPage, newPage0, newPage1, 1.0f, false);

            scene.leftPageNo = newPage0;
        }

        if (input.KeyDown(LLGL::Key::Right) && !ReverseLastPage(false))
        {
            int oldPage     = scene.leftPageNo;
            int newPage0    = ModuloSignInt(scene.leftPageNo - 1, numPages);
            int newPage1    = ModuloSignInt(scene.leftPageNo - 2, numPages);

            AnimatePage(newPage0, oldPage, newPage1, 1.0f, true);

            scene.leftPageNo = newPage1;
        }
    }

    void UpdateScene(float dt)
    {
        UpdateUserInput();

        const float projZAxis = GetProjectionZAxis();

        // Update view-projection matrix
        Gs::Matrix4f vMatrix;
        vMatrix.LoadIdentity();
        Gs::Translate(vMatrix, Gs::Vector3f{ 0, 3.0f, -3.0f });
        Gs::RotateX(vMatrix, Gs::Deg2Rad(45.0f));
        vMatrix.MakeInverse();

        sceneView.wvpMatrix = projection * vMatrix;

        sceneView.wMatrix.LoadIdentity();

        // Advance all animations and remove those that have finished
        animations.erase(
            std::remove_if(
                animations.begin(),
                animations.end(),
                [dt](MorphTargetAnimation& anim) -> bool
                {
                    // Advance animation state and remove element once stopped
                    anim.Animate(dt);
                    return !anim.IsPlaying();
                }
            ),
            animations.end()
        );
    }

    void DrawStaticMesh(const StaticMesh& mesh)
    {
        commands->SetResource(BindingTable_ColorMap, *(mesh.colorMap));
        commands->Draw(mesh.numVertices, mesh.firstVertex);
    }

    void RenderStaticMeshes()
    {
        // Bind PSO for static meshes and scene view buffer
        commands->SetPipelineState(*psoStaticMesh);
        commands->SetResource(BindingTable_SceneView, *sceneViewCbuffer);

        // All static meshes are stored in the first section of the mesh buffer.
        // So use the default stride and the begining (offset 0) as vertex buffer.
        commands->SetVertexBuffer(*meshBuffer);

        // Draw book shell
        DrawStaticMesh(scene.meshBookShell);

        // Draw resting left page
        DrawStaticMesh(scene.meshLRestingPage);

        // Draw resting right page
        DrawStaticMesh(scene.meshRRestingPage);
    }

    void DrawMorphTargetMesh(const MorphTargetMesh& mesh, const MorphTargetAnimation& anim, PageFace pageFace)
    {
        if (mesh.numVertices == 0 || anim.numKeyframes == 0 || anim.nextKeyframe >= mesh.keyframeVbufferOffset.size())
            return;

        // Bind all morph-target vertex buffers for current and next keyframe
        LLGL::VertexBufferView morphTargetVbufferViews[MTVB_Count];
        {
            morphTargetVbufferViews[MTVB_CurrentTarget].buffer  = meshBuffer;
            morphTargetVbufferViews[MTVB_CurrentTarget].stride  = sizeof(MorphTargetKeyframeVertex);
            morphTargetVbufferViews[MTVB_CurrentTarget].offset  = mesh.keyframeVbufferOffset[anim.currentKeyframe];

            morphTargetVbufferViews[MTVB_NextTarget].buffer     = meshBuffer;
            morphTargetVbufferViews[MTVB_NextTarget].stride     = sizeof(MorphTargetKeyframeVertex);
            morphTargetVbufferViews[MTVB_NextTarget].offset     = mesh.keyframeVbufferOffset[anim.nextKeyframe];

            morphTargetVbufferViews[MTVB_TexCoords].buffer      = meshBuffer;
            morphTargetVbufferViews[MTVB_TexCoords].stride      = sizeof(Gs::Vector2f);
            morphTargetVbufferViews[MTVB_TexCoords].offset      = mesh.texCoordsVbufferOffset;
        }
        commands->SetVertexBuffers(MTVB_Count, morphTargetVbufferViews);

        // Update animation state to interpolate between the two keyframes
        commands->SetUniforms(BindingTable_AnimationState, &(anim.interpolationFactor), sizeof(anim.interpolationFactor));

        float invertXAxis = (anim.IsMovingForward() ? 1.0f : -1.0f);
        commands->SetUniforms(BindingTable_InvertXAxis, &invertXAxis, sizeof(invertXAxis));

        // Bind page texture
        commands->SetResource(BindingTable_ColorMap, *(anim.faceTextures[pageFace]));

        // Draw mesh
        commands->Draw(mesh.numVertices, 0);
    }

    void RenderMorphTargetMeshesFace(PageFace pageFace)
    {
        // Bind PSO for morph-targets and scene view buffer
        commands->SetPipelineState(*psoMorphTargetMesh[pageFace]);
        commands->SetResource(BindingTable_SceneView, *sceneViewCbuffer);

        // Draw all animated moving pages
        for (MorphTargetAnimation& anim : animations)
            DrawMorphTargetMesh(scene.meshMovingPage, anim, pageFace);
    }

    void RenderMorphTargetMeshes()
    {
        commands->PushDebugGroup("Front Faces");
        RenderMorphTargetMeshesFace(PageFace_Front);
        commands->PopDebugGroup();

        commands->PushDebugGroup("Back Faces");
        RenderMorphTargetMeshesFace(PageFace_Back);
        commands->PopDebugGroup();
    }

    void OnDrawFrame(float dt) override
    {
        // Update scene by user input
        UpdateScene(dt);

        commands->Begin();
        {
            commands->UpdateBuffer(*sceneViewCbuffer, 0, &sceneView, sizeof(sceneView));

            // Render scene onto screen
            commands->BeginRenderPass(*swapChain);
            {
                commands->Clear(LLGL::ClearFlags::ColorDepth, backgroundColor);
                commands->SetViewport(swapChain->GetResolution());

                commands->PushDebugGroup("Static Mesh Pass");
                RenderStaticMeshes();
                commands->PopDebugGroup();

                commands->PushDebugGroup("Morph-Target Mesh Pass");
                RenderMorphTargetMeshes();
                commands->PopDebugGroup();
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_MorphTargets);



