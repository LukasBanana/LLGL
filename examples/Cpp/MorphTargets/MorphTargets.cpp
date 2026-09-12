/*
 * MorphTargets.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <LLGL/Platform/Platform.h>
#include <functional>
#include <chrono>



static int ModuloSignInt(int a, int b)
{
    return (((a % b + b)) % b);
}

static float ModuloSignFloat(float value, int& intPart)
{
    float floorValue = std::floor(value);
    intPart = static_cast<int>(floorValue);
    return value - floorValue;
}

// Helper function to calculate modulo operator with signed values to wrap around start and end keyframes.
static std::uint32_t ModuloSubtract(std::uint32_t lhs, std::uint32_t rhs, std::uint32_t wrap)
{
    int a = static_cast<int>(lhs) - static_cast<int>(rhs);
    int b = static_cast<int>(wrap);
    return static_cast<std::uint32_t>(ModuloSignInt(a, b));
}

// This example renders an open book and animates the pages via morph-target animation,
// a set of keyframe meshes that are interpolated in the vertex shader.
class Example_MorphTargets : public ExampleBase
{

    static constexpr float      pageTurningTime             = 2.0f;     // Time to turn a page in seconds
    static constexpr int        pageTurningMouseThreshold   = 10;       // How many pixels has the mouse to move before starting to turn a page?
    static constexpr float      pageTurningMouseSpeed       = 0.01f;
    static constexpr float      mouseGrappleMomentum        = 0.01f;    // Mouse motion required to keep a flicked page going
    static constexpr int        numBookPageKeyframes        = 16;       // Number of keyframe meshes to load from 'BookPageKeyframe_<N>.obj' pattern
    static constexpr long long  nudgePageWaitTime           = 3000;     // Time to nudge a page when there's no animation, to grab user's attention (in milliseconds)
    static constexpr float      nudgePageSpeed              = 2.0f;

    // Zero-based binding slots for vertex input buffers.
    // This example uses three vertex buffers simultaneously for the morph-target animations:
    // (1) current frame vertices, (2) next frame vertices, (3) shared texture coordiantes.
    enum MorphTargetVertexBuffer
    {
        MTVB_CurrentTarget = 0,
        MTVB_NextTarget,
        MTVB_TexCoords,

        MTVB_Count,
    };

    // Enumeration with zero-based indices for resources in the pipeline layouts. These are used to bind resources via `SetResource()`.
    enum BindingTable
    {
        // Resources:
        BindingTable_SceneView          = 0,

        BindingTable_PaperDetailMap,
        BindingTable_ColorMap,
        BindingTable_ColorMapSampler,

        BindingTable_FrontPageTexture   = BindingTable_ColorMap,
        BindingTable_FrontPageSampler,
        BindingTable_BackPageTexture,
        BindingTable_BackPageSampler,

        // Uniforms:
        BindingTable_TexCoordScaleFront = 0,
        BindingTable_BorderSamplerFront,
    };

    // Array index for all sampler states used to render pages in different presentations.
    enum SamplerId
    {
        SamplerId_Default = 0,
        SamplerId_Wrap,
        SamplerId_Mirror,
        SamplerId_BorderBlack,
        SamplerId_LodBias,
        SamplerId_LodBiasNearest,

        SamplerId_Count,
    };

    // GLES and WebGL don't support GL_CLAMP_TO_BORDER, so we emulate it in the shader
    enum BorderSampler
    {
        BorderSampler_None = 0,
        BorderSampler_BlackTransparent,
        BorderSampler_Black,
    };

    // Direction a page can be turned.
    enum class PageTurnDirection
    {
        Left    = -1,
        None    = 0,
        Right   = +1,
    };

    // Specifies the modes the mouse grappling mechanism can be in.
    enum class MouseGrappling
    {
        Inactive,
        Starting,
        Active,
    };

    // Type of animation actors, whether the page is turned by keyboard input, mouse input, or nudged to grab the user's attention.
    enum class AnimationActor
    {
        Kinetic,    // Animation triggered by keyboard input
        Grappled,   // Animation triggered by mouse grappling
        Nudged,     // Animation triggered by nudging the page and letting it fall back to grab user's attention
    };

    // Minimalistic material struct with just a texture, sampler, and some metadata for shader uniforms.
    struct Material
    {
        LLGL::Texture*  colorMap        = nullptr;
        LLGL::Sampler*  colorMapSampler = nullptr;
        float           texScale        = 1.0f;
        BorderSampler   borderSampler   = BorderSampler_None;
    };

    // Vertex buffer range and material information for a static mesh. Used for the book shell and the resting left and right pages.
    struct StaticMesh
    {
        std::uint32_t   numVertices     = 0;
        std::uint32_t   firstVertex     = 0;
        Material        material;
    };

    // Vertex for morph-target keyframes.
    // A morph-target animated mesh is rendered with the current and next keyframe,
    // interpolates between them, and the shared texture-coordiantes.
    struct MorphTargetKeyframeVertex
    {
        Gs::Vector3f position;
        Gs::Vector3f normal;
    };

    // Keyframe and shared texture-coordinate buffer range for morph-target animated meshes.
    struct MorphTargetMesh
    {
        std::uint32_t               numVertices             = 0;
        std::uint64_t               texCoordsVbufferOffset  = 0;
        std::vector<std::uint64_t>  keyframeVbufferOffset;
    };

    // Function signature when a page is finished animating.
    using AnimFinishCallback = std::function<void(PageTurnDirection direction)>;

    // Tracks animation state and the page to render in the 3D book model.
    struct MorphTargetAnimation
    {
        std::uint32_t       numKeyframes        = 0;
        std::uint32_t       currentKeyframe     = 0;
        std::uint32_t       nextKeyframe        = 0;
        float               interpolationFactor = 0.0f;
        float               frameSpeed          = 0.0f;
        AnimationActor      actor               = AnimationActor::Kinetic;  // What caused the animation: Keyboard, Mouse, or Nudged to grab user's attention?
        bool                isReverse           = false;                    // Animate with reversed X-axis transformation
        PageTurnDirection   direction           = PageTurnDirection::None;
        Material            faceMaterials[2];
        AnimFinishCallback  finishCallback;

        PageTurnDirection GetDirectionForMotion(float motion) const
        {
            if (motion > 0.0f)
                return isReverse ? PageTurnDirection::Right : PageTurnDirection::Left;
            if (motion < 0.0f)
                return isReverse ? PageTurnDirection::Left : PageTurnDirection::Right;
            return PageTurnDirection::None;
        }

        void SetFrameSpeed(float animTime)
        {
            frameSpeed = static_cast<float>(numKeyframes) / animTime;
        }

        void Play(
            Material frontMaterial, Material backMaterial, std::uint32_t keyframes,
            float animTime = 1.0f, bool reverse = false, AnimationActor actor = AnimationActor::Kinetic, const AnimFinishCallback& callback = nullptr)
        {
            if (keyframes >= 2)
            {
                this->numKeyframes          = keyframes;
                this->interpolationFactor   = 0.0f;
                this->SetFrameSpeed(animTime);
                this->actor                 = actor;
                this->isReverse             = reverse;
                this->currentKeyframe       = 0;
                this->nextKeyframe          = 1;
                this->faceMaterials[1]      = frontMaterial;
                this->faceMaterials[0]      = backMaterial;
                this->finishCallback        = callback;
                this->direction             = GetDirectionForMotion(frameSpeed);
            }
        }

        // Invokes the OnFinished() callback and resets animation state.
        void Stop()
        {
            OnFinished();
            direction       = PageTurnDirection::None;
            frameSpeed      = 0.0f;
            nextKeyframe    = currentKeyframe;
        }

        // Returns true if the animation is actively playing.
        bool IsPlaying() const
        {
            return (nextKeyframe != currentKeyframe);
        }

        // Invokes the callback to signal the end of the animation. This callback can be specified when staring to play the animation.
        // This is used to signal when the resting left or right page of the book have to change their texture.
        void OnFinished()
        {
            if (finishCallback)
                finishCallback(direction);
        }

        // Advances the animation by the specified delta-time (dt) value.
        // This parameter should be the time for a single frame (e.g. 16ms for a 60 FPS game) for kentic animation
        // or the mouse motion when the mouse grappling mechanism is active.
        void Animate(float dt)
        {
            // Update direction the page is moving to, in case the direction has changed
            // half way throgh the animation cycle when the user is flicking through pages with the mouse.
            direction = GetDirectionForMotion(dt * frameSpeed);

            if (IsPlaying())
            {
                int advanceFrames = 0;
                interpolationFactor = ModuloSignFloat(interpolationFactor + dt * frameSpeed, advanceFrames);

                if (advanceFrames > 0)
                {
                    const std::uint32_t advanceFramesAbs = static_cast<std::uint32_t>(advanceFrames);
                    if (currentKeyframe + advanceFramesAbs + 1 < numKeyframes)
                    {
                        currentKeyframe += advanceFramesAbs;
                        nextKeyframe    = currentKeyframe + 1;
                    }
                    else
                        Stop();
                }
                else if (advanceFrames < 0)
                {
                    const std::uint32_t advanceFramesAbs = static_cast<std::uint32_t>(-advanceFrames);
                    if (currentKeyframe >= advanceFramesAbs)
                    {
                        currentKeyframe -= advanceFramesAbs;
                        nextKeyframe    = currentKeyframe + 1;
                    }
                    else
                        Stop();
                }
            }
        }

        // Tries to flick a page with enough mouse grappling movement by turning it into a kinetic actor.
        void TryToFlickPage(float mouseGrappleMovement)
        {
            if (actor == AnimationActor::Grappled)
            {
                // If the animation was previously grappled by the mouse, check if the animation passed the 'point of no return' to let it finish.
                // Otherwise, roll back animation to let the page fall back into its original place.
                if ((currentKeyframe < numKeyframes/2) &&
                    !((direction == PageTurnDirection::Left  && mouseGrappleMovement > +mouseGrappleMomentum) ||
                      (direction == PageTurnDirection::Right && mouseGrappleMovement < -mouseGrappleMomentum)))
                {
                    frameSpeed = -frameSpeed;
                }

                // Turn the animatiom back to kinetic actor as the mouse lets go of the page
                actor = AnimationActor::Kinetic;
            }
        }
    };

    // Constant buffer data for the scene view. Constant buffer data should always be 16 byte aligned to ensure each backend uploads its data correctly.
    struct alignas(16) SceneView
    {
        Gs::Matrix4f        wvpMatrix;
        Gs::Matrix4f        wMatrix;
        Gs::Vector3f        lightVec    = -Gs::Vector3f(-0.25f, -1.0f, 0.5f).Normalized();
        float               pad0;
        LLGL::ColorRGBAf    baseColor   = { 1.0f, 1.0f, 1.0f, 1.0f };
    }
    sceneView;

    // Scene model data for this example. Contains all mesh information to render the scene.
    struct Scene
    {
        StaticMesh      meshBookShell;
        StaticMesh      meshLRestingPage;
        StaticMesh      meshRRestingPage;
        MorphTargetMesh meshMovingPage;
    }
    scene;

    // Tracks the state of turning pages.
    struct PageTurning
    {
        std::int32_t                            mouseStartPosX      = 0;

        // Store separately what page numbers are currently visible on the left and the right of the book
        // as the user can flick through multiple pages at once.
        int                                     leftPageNo          = 0;
        int                                     rightPageNo         = 1;

        MouseGrappling                          mouseGrappling      = MouseGrappling::Inactive;
        std::chrono::system_clock::time_point   timeSinceNoAnims    = {};
    }
    pageTurning;

    std::vector<Material>               pages;                                          // List of materials for all book pages to render
    std::vector<MorphTargetAnimation>   animations;                                     // List of all active animations

    LLGL::Shader*                       vsStaticMesh                        = nullptr;
    LLGL::Shader*                       fsStaticMesh                        = nullptr;

    LLGL::Shader*                       vsMorphTargetMesh                   = nullptr;
    LLGL::Shader*                       fsMorphTargetMesh                   = nullptr;

    LLGL::PipelineLayout*               psoLayoutStatic                     = nullptr;
    LLGL::PipelineLayout*               psoLayoutMorphTarget                = nullptr;

    LLGL::PipelineState*                psoStaticMesh                       = nullptr;
    LLGL::PipelineState*                psoMorphTargetMesh                  = nullptr;

    LLGL::Buffer*                       meshBuffer                          = nullptr;  // Single mesh buffer containing vertex data for the entire scene
    LLGL::Buffer*                       sceneViewCbuffer                    = nullptr;  // Scene view constant buffer

    LLGL::Texture*                      bookShellTexture                    = nullptr;
    LLGL::Texture*                      bookPaperDetailMap                  = nullptr;

    LLGL::Sampler*                      textureSamplers[SamplerId_Count]    = {};

public:

    Example_MorphTargets() :
        ExampleBase { "LLGL Example: MorphTargets" }
    {
        // Create all graphics objects
        LoadMaterials();
        LoadMeshes();
        CreateStaticMeshPSO();
        CreateMorphTargetMeshPSO();

        // Update vectors for projection
        sceneView.lightVec.z *= GetProjectionZAxis();

        // Initialize timers
        pageTurning.timeSinceNoAnims = std::chrono::system_clock::now();
    }

private:

    void LoadMaterials()
    {
        // Create texture samplers. GLES and WebGL don't support clamp-to-border, so we use clamp-to-edge instead
        if (renderer->GetRendererID() == LLGL::RendererID::OpenGLES || renderer->GetRendererID() == LLGL::RendererID::WebGL)
        {
            textureSamplers[SamplerId_Default]          = renderer->CreateSampler(LLGL::Parse("address.uvw=clamp"));
            textureSamplers[SamplerId_BorderBlack]      = textureSamplers[SamplerId_Default];
            textureSamplers[SamplerId_LodBias]          = renderer->CreateSampler(LLGL::Parse("lod.min=4"));
            textureSamplers[SamplerId_LodBiasNearest]   = renderer->CreateSampler(LLGL::Parse("lod.min=4,lod.max=4,filter=nearest"));
        }
        else
        {
            textureSamplers[SamplerId_Default]          = renderer->CreateSampler(LLGL::Parse("address.uvw=border"));
            textureSamplers[SamplerId_BorderBlack]      = renderer->CreateSampler(LLGL::Parse("address.uvw=border,border=black"));
            textureSamplers[SamplerId_LodBias]          = renderer->CreateSampler(LLGL::Parse("address.uvw=border,lod.bias=3"));
            textureSamplers[SamplerId_LodBiasNearest]   = renderer->CreateSampler(LLGL::Parse("address.uvw=border,lod.min=4,lod.max=4,filter=nearest"));
        }

        textureSamplers[SamplerId_Wrap]     = renderer->CreateSampler({}); // Linear sampler with default address mode
        textureSamplers[SamplerId_Mirror]   = renderer->CreateSampler(LLGL::Parse("address.uvw=mirror"));

        // Load textures
        bookShellTexture = LoadTexture("Book/Book.png");
        bookPaperDetailMap = LoadTexture("Book/Book_PaperDetailMap.png");

        LLGL::Texture* textures[] =
        {
            LoadTexture("Book/Book_Page0.png"),
            LoadTexture("Logos/Logo_LLGL.png"),
            LoadTexture("Crate.jpg"),
            LoadTexture("Logos/Logo_Direct3D12.png"),
            LoadTexture("Logos/Logo_Direct3D11.png"),
            LoadTexture("Logos/Logo_Vulkan.png"),
            LoadTexture("Logos/Logo_OpenGL.png"),
            LoadTexture("Logos/Logo_Metal.png"),
            LoadTexture("Logos/Logo_LLGL.png"),
        };

        auto MakeMaterial = [this, &textures](int texId, float texScale = 1.0f, SamplerId texSamplerId = SamplerId_Default, BorderSampler borderSampler = BorderSampler_BlackTransparent) -> Material
        {
            Material outMaterial;
            outMaterial.colorMap        = textures[texId];
            outMaterial.colorMapSampler = this->textureSamplers[texSamplerId];
            outMaterial.texScale        = texScale;
            outMaterial.borderSampler   = borderSampler;
            return outMaterial;
        };

        pages =
        {
            MakeMaterial(0),
            MakeMaterial(1),
            MakeMaterial(2, 1.50f, SamplerId_BorderBlack, BorderSampler_Black),
            MakeMaterial(2, 1.50f, SamplerId_Default),
            MakeMaterial(2, 1.50f, SamplerId_LodBias),
            MakeMaterial(2, 1.50f, SamplerId_LodBiasNearest),
            MakeMaterial(2, 5.00f, SamplerId_Wrap, BorderSampler_None),
            MakeMaterial(2, 5.00f, SamplerId_Mirror, BorderSampler_None),
            MakeMaterial(3, 1.25f, SamplerId_Default),
            MakeMaterial(4, 1.25f, SamplerId_Default),
            MakeMaterial(5, 1.25f, SamplerId_Default),
            MakeMaterial(6, 1.25f, SamplerId_Default),
            MakeMaterial(7, 1.25f, SamplerId_Default),
            MakeMaterial(8),
        };
    }

    // Loads a static mesh from the specified file and stores its vertices in the specified output container.
    StaticMesh LoadStaticMesh(std::vector<TexturedVertex>& outVertices, const std::string& filename, const Material& material)
    {
        TriangleMesh intermediateMesh = Load3DModel(outVertices, filename, 3, MeshFlags_FlipTexCoordV);

        StaticMesh outMesh;
        {
            outMesh.numVertices = intermediateMesh.numVertices;
            outMesh.firstVertex = intermediateMesh.firstVertex;
            outMesh.material    = material;
        }
        return outMesh;
    }

    // Helper function that returns the size (in bytes) of the specified STL container.
    template <typename TContainer>
    static std::size_t ByteSize(const TContainer& cont)
    {
        return sizeof(typename TContainer::value_type) * cont.size();
    }

    // Helper function to retrieve the 3D model filename of the specified keyframe.
    static std::string KeyframeMeshFilename(int keyframe)
    {
        return ("Book/BookPageKeyframe_" + std::to_string(keyframe) + ".obj");
    }

    void LoadMeshes()
    {
        LLGL_VERIFY(numBookPageKeyframes >= 2 && "Need at least 2 keyframes to load");
        LLGL_VERIFY(bookShellTexture != nullptr);
        LLGL_VERIFY(pages.size() >= pageTurning.leftPageNo && pages.size() >= pageTurning.rightPageNo);
        LLGL_VERIFY(pages[pageTurning.leftPageNo].colorMap != nullptr);
        LLGL_VERIFY(pages[pageTurning.rightPageNo].colorMap != nullptr);

        Material bookShellMaterial;
        bookShellMaterial.colorMap          = bookShellTexture;
        bookShellMaterial.colorMapSampler   = textureSamplers[SamplerId_Default];

        // Load static meshes
        std::vector<TexturedVertex> staticVertices;
        scene.meshBookShell     = LoadStaticMesh(staticVertices, "Book/BookShell.obj", bookShellMaterial);
        scene.meshLRestingPage  = LoadStaticMesh(staticVertices, "Book/BookLeftPage.obj", pages[pageTurning.leftPageNo]);
        scene.meshRRestingPage  = LoadStaticMesh(staticVertices, "Book/BookRightPage.obj", pages[pageTurning.rightPageNo]);

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

    void CreateStaticMeshPSO()
    {
        // Create PSO layout
        psoLayoutStatic = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneView@3):vert:frag,"

                "texture(paperDetailMap@4):frag,"
                "sampler(paperDetailMapSampler@5){}:frag," // Static sampler

                "texture(colorMap@6):frag,"
                "sampler(colorMapSampler@7):frag," // Dynamic sampler

                "float(dynamicState.texCoordScaleFront),"
                "int(dynamicState.borderSamplerFront),"

                "sampler<paperDetailMap, paperDetailMapSampler>(s_paperDetailMappaperDetailMapSampler@4),"
                "sampler<colorMap, colorMapSampler>(s_colorMapcolorMapSampler@6),"
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
            psoStaticMeshDesc.vertexShader                      = LoadVertexShader  ("MorphTargets", "VStaticMesh");
            psoStaticMeshDesc.fragmentShader                    = LoadFragmentShader("MorphTargets", "PStaticMesh");
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

                "texture(paperDetailMap@4):frag,"
                "sampler(paperDetailMapSampler@5){}:frag," // Static sampler

                "texture(frontPageColorMap@6):frag,"
                "sampler(frontPageSampler@7):frag," // Dynamic sampler
                "texture(backPageColorMap@8):frag,"
                "sampler(backPageSampler@9):frag," // Dynamic sampler

                "float(dynamicState.texCoordScaleFront),"
                "float(dynamicState.texCoordScaleBack),"
                "float(dynamicState.interpolationFactor)," // Interpolation factor as uniform to efficiently animate many morph targets
                "float(dynamicState.invertXAxis),"
                "int(dynamicState.borderSamplerFront),"
                "int(dynamicState.borderSamplerBack),"

                "sampler<paperDetailMap, paperDetailMapSampler>(s_paperDetailMappaperDetailMapSampler@4),"
                "sampler<frontPageColorMap, frontPageSampler>(s_frontPageColorMapfrontPageSampler@6),"
                "sampler<backPageColorMap, backPageSampler>(s_backPageColorMapbackPageSampler@8),"
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

        // Create graphics pipeline for morph-target animation
        LLGL::GraphicsPipelineDescriptor psoMorphTargetDesc;
        {
            psoMorphTargetDesc.inputVertexAttribs                   = morphTargetMeshVertexAttribs;
            psoMorphTargetDesc.vertexShader                         = LoadVertexShader  ("MorphTargets", "VMorphTargetMesh");
            psoMorphTargetDesc.fragmentShader                       = LoadFragmentShader("MorphTargets", "PMorphTargetMesh");
            psoMorphTargetDesc.renderPass                           = swapChain->GetRenderPass();
            psoMorphTargetDesc.pipelineLayout                       = psoLayoutMorphTarget;
            psoMorphTargetDesc.depth.testEnabled                    = true;
            psoMorphTargetDesc.depth.writeEnabled                   = true;
            psoMorphTargetDesc.rasterizer.cullMode                  = LLGL::CullMode::Disabled;
            psoMorphTargetDesc.rasterizer.multiSampleEnabled        = (GetSampleCount() > 1);

            // Render animated pages with a depth offset as they will be placed very close to the static mesh
            psoMorphTargetDesc.rasterizer.depthBias.constantFactor  = -100.0f;
        }
        psoMorphTargetMesh = renderer->CreatePipelineState(psoMorphTargetDesc);
        ReportPSOErrors(psoMorphTargetMesh);
    }

    const Material& GetPageMaterial(int pageNo) const
    {
        const int numPages = static_cast<int>(pages.size());
        const int pageTextureIndex = ModuloSignInt(pageNo, numPages);
        return pages[pageTextureIndex];
    }

    // Sets the texture and tracking number of the resting left page.
    void SetLeftPage(int page)
    {
        scene.meshLRestingPage.material = GetPageMaterial(page);
        pageTurning.leftPageNo = page;
    }

    // Sets the texture and tracking number of the resting right page.
    void SetRightPage(int page)
    {
        scene.meshRRestingPage.material = GetPageMaterial(page);
        pageTurning.rightPageNo = page;
    }

    // Starts the animation of turning a page.
    void AnimatePage(int frontPage, int backPage, int revealedPage, float animTime = 1.0f, bool reverse = false, AnimationActor actor = AnimationActor::Kinetic)
    {
        MorphTargetAnimation anim;
        const std::uint32_t numKeyframes = static_cast<std::uint32_t>(scene.meshMovingPage.keyframeVbufferOffset.size());

        anim.Play(
            GetPageMaterial(frontPage),
            GetPageMaterial(backPage),
            numKeyframes,
            animTime,
            reverse,
            actor,
            [this, backPage, frontPage](PageTurnDirection direction)
            {
                // Replace the page texture (and number) in the book with this moving page having finished animating.
                if (direction == PageTurnDirection::Left)
                    this->SetLeftPage(backPage);
                else if (direction == PageTurnDirection::Right)
                    this->SetRightPage(frontPage);
                this->ResetMouseGrappling();
            }
        );

        // Set the newly revealed page texture
        if (anim.direction == PageTurnDirection::Left)
            SetRightPage(revealedPage);
        else if (anim.direction == PageTurnDirection::Right)
            SetLeftPage(revealedPage);

        animations.push_back(anim);
    }

    // Tries to flick back the last turned page if its still actively being animated and the direction matches the new input direction (for kinetic actors).
    bool TryFlickBackLastPage(PageTurnDirection direction, AnimationActor actor)
    {
        for (auto it = animations.rbegin(); it != animations.rend(); ++it)
        {
            MorphTargetAnimation& anim = *it;
            if (actor == AnimationActor::Grappled || (anim.actor != AnimationActor::Grappled && anim.direction != direction))
            {
                // Grapple the page by the mouse or otherwise reverse its direction by inverting the animation speed.
                // Don't use the `anim.isReverse` property as this would mirror the animation, but this should only reverse the motion.
                if (actor == AnimationActor::Grappled)
                {
                    anim.actor = AnimationActor::Grappled;
                    anim.SetFrameSpeed(pageTurningTime);
                }
                else
                    anim.frameSpeed = -anim.frameSpeed;
                return true;
            }
        }
        return false;
    }

    // Kicks off the state tracking and animation to turn a page into the specified direction.
    void TurnPage(PageTurnDirection direction, AnimationActor actor = AnimationActor::Kinetic, float animTime = pageTurningTime)
    {
        struct PageSet
        {
            int frontPage       = 0;
            int backPage        = 0;
            int revealedPage    = 0;
        };

        if (!TryFlickBackLastPage(direction, actor))
        {
            const int numPages = static_cast<int>(pages.size());

            PageSet pageSet;
            if (direction == PageTurnDirection::Left)
            {
                pageSet.frontPage       = pageTurning.rightPageNo;
                pageSet.backPage        = ModuloSignInt(pageTurning.rightPageNo + 1, numPages);
                pageSet.revealedPage    = ModuloSignInt(pageTurning.rightPageNo + 2, numPages);
            }
            else if (direction == PageTurnDirection::Right)
            {
                // Front and back pages are flipped here, because the animation plays in reverse,
                // but front and back are always the same in the geometry.
                pageSet.backPage        = pageTurning.leftPageNo;
                pageSet.frontPage       = ModuloSignInt(pageTurning.leftPageNo - 1, numPages);
                pageSet.revealedPage    = ModuloSignInt(pageTurning.leftPageNo - 2, numPages);
            }
            else
            {
                return;
            }

            const bool animateInReverse = (direction == PageTurnDirection::Right);
            AnimatePage(pageSet.frontPage, pageSet.backPage, pageSet.revealedPage, animTime, animateInReverse, actor);
        }

        // If mouse grappling started, it's now active
        if (actor == AnimationActor::Grappled)
            pageTurning.mouseGrappling = MouseGrappling::Active;
    }

    // Restes state tracking for the mouse grappling mechanism.
    // Call this at the beginning of pressing the mouse button and at the end of a page animation that was controlled by the mouse.
    void ResetMouseGrappling()
    {
        if (pageTurning.mouseGrappling != MouseGrappling::Inactive)
            pageTurning.mouseStartPosX = input.GetMousePosition().x;
    }

    // Helper function to retrieve the motion for the mouse grappling mechanism.
    float GetMouseGrappleMovement() const
    {
        return static_cast<float>(-input.GetMouseMotion().x) * pageTurningMouseSpeed;
    }

    void UpdateUserInput()
    {
        // When no page is being grappled by the mouse, the keyboard can trigger kinetic page movement
        if (pageTurning.mouseGrappling == MouseGrappling::Inactive)
        {
            if (input.KeyDown(LLGL::Key::Left))
                TurnPage(PageTurnDirection::Left);
            if (input.KeyDown(LLGL::Key::Right))
                TurnPage(PageTurnDirection::Right);
        }

        if (input.KeyDown(LLGL::Key::LButton))
        {
            pageTurning.mouseGrappling = MouseGrappling::Starting;
            ResetMouseGrappling();
        }
        else if (input.KeyUp(LLGL::Key::LButton))
        {
            const float mouseGrappleMovement = GetMouseGrappleMovement();

            // Continue animation for all active animations without grappling, allowing the user to 'flick through pages'.
            for (MorphTargetAnimation& anim : animations)
                anim.TryToFlickPage(mouseGrappleMovement);

            pageTurning.mouseGrappling = MouseGrappling::Inactive;
        }

        if (pageTurning.mouseGrappling == MouseGrappling::Active)
        {
            if (animations.empty())
                pageTurning.mouseGrappling = MouseGrappling::Starting;
        }

        if (pageTurning.mouseGrappling == MouseGrappling::Starting)
        {
            const std::int32_t mouseDiffX = input.GetMousePosition().x - pageTurning.mouseStartPosX;

            if (mouseDiffX > pageTurningMouseThreshold)
                TurnPage(PageTurnDirection::Right, AnimationActor::Grappled);
            else if (mouseDiffX < -pageTurningMouseThreshold)
                TurnPage(PageTurnDirection::Left, AnimationActor::Grappled);
        }
    }

    void UpdateScene(float dt)
    {
        UpdateUserInput();

        const float projZAxis = GetProjectionZAxis();

        // Nudge a page if there have been no animations for some time
        auto currentTime = std::chrono::system_clock::now();
        if (animations.empty())
        {
            auto elapsedTimeSinceNoAnims = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - pageTurning.timeSinceNoAnims).count();
            if (elapsedTimeSinceNoAnims > nudgePageWaitTime)
            {
                TurnPage(PageTurnDirection::Left, AnimationActor::Nudged, pageTurningTime / nudgePageSpeed);
                pageTurning.timeSinceNoAnims = currentTime;
            }
        }
        else
            pageTurning.timeSinceNoAnims = currentTime;

        // Update view-projection matrix
        Gs::Matrix4f vMatrix;
        vMatrix.LoadIdentity();
        Gs::Translate(vMatrix, Gs::Vector3f{ 0, 3.0f, -3.0f });
        Gs::RotateX(vMatrix, Gs::Deg2Rad(45.0f));
        vMatrix.MakeInverse();

        sceneView.wvpMatrix = projection * vMatrix;

        sceneView.wMatrix.LoadIdentity();

        // Advance all animations and remove those that have finished
        const float mouseGrappleMovement = GetMouseGrappleMovement();

        animations.erase(
            std::remove_if(
                animations.begin(),
                animations.end(),
                [dt, mouseGrappleMovement](MorphTargetAnimation& anim) -> bool
                {
                    // Advance animation state and remove element once stopped
                    if (anim.actor == AnimationActor::Nudged)
                    {
                        anim.frameSpeed -= dt * nudgePageSpeed * static_cast<float>(anim.numKeyframes);
                        anim.Animate(dt);
                    }
                    else if (anim.actor == AnimationActor::Grappled)
                    {
                        anim.Animate(anim.isReverse ? -mouseGrappleMovement : mouseGrappleMovement);
                    }
                    else
                    {
                        anim.Animate(dt);
                    }
                    return !anim.IsPlaying();
                }
            ),
            animations.end()
        );
    }

    void BindMaterial(std::uint32_t firstDescriptor, const Material& material)
    {
        commands->SetResource(firstDescriptor, *(material.colorMap));
        commands->SetResource(firstDescriptor + 1, *(material.colorMapSampler));
    }

    void DrawStaticMesh(const StaticMesh& mesh)
    {
        BindMaterial(BindingTable_ColorMap, mesh.material);

        struct StaticMeshDynamicState
        {
            float           texCoordScale;
            std::int32_t    borderSampler;
        }
        dynamicState =
        {
            mesh.material.texScale,
            mesh.material.borderSampler
        };
        commands->SetUniforms(0, &dynamicState, sizeof(dynamicState));

        commands->Draw(mesh.numVertices, mesh.firstVertex);
    }

    void RenderStaticMeshes()
    {
        // Bind PSO for static meshes and scene view buffer
        commands->SetPipelineState(*psoStaticMesh);

        commands->SetResource(BindingTable_SceneView, *sceneViewCbuffer);
        commands->SetResource(BindingTable_PaperDetailMap, *bookPaperDetailMap);

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

    void DrawMorphTargetMesh(const MorphTargetMesh& mesh, const MorphTargetAnimation& anim)
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
        struct MorphTargetDynamicState
        {
            float           texCoordScales[2];
            float           interpolationFactor;
            float           invertXAxis;
            std::int32_t    borderSamplers[2];
        }
        dynamicState =
        {
            { anim.faceMaterials[0].texScale, anim.faceMaterials[1].texScale },
            anim.interpolationFactor,
            (anim.isReverse ? -1.0f : +1.0f),
            { anim.faceMaterials[0].borderSampler, anim.faceMaterials[1].borderSampler },
        };
        commands->SetUniforms(0, &dynamicState, sizeof(dynamicState));

        // Bind materials (texture+sampler) for front and back faces of the page
        BindMaterial(BindingTable_FrontPageTexture, anim.faceMaterials[0]);
        BindMaterial(BindingTable_BackPageTexture, anim.faceMaterials[1]);

        // Draw mesh
        commands->Draw(mesh.numVertices, 0);
    }

    void RenderMorphTargetMeshes()
    {
        // Bind PSO for morph-targets and scene view buffer
        commands->SetPipelineState(*psoMorphTargetMesh);

        commands->SetResource(BindingTable_SceneView, *sceneViewCbuffer);
        commands->SetResource(BindingTable_PaperDetailMap, *bookPaperDetailMap);

        // Draw all animated moving pages
        for (MorphTargetAnimation& anim : animations)
            DrawMorphTargetMesh(scene.meshMovingPage, anim);
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

// Clang/GCC need these declared here as well prior to C++17
constexpr float     Example_MorphTargets::pageTurningTime          ;
constexpr int       Example_MorphTargets::pageTurningMouseThreshold;
constexpr float     Example_MorphTargets::pageTurningMouseSpeed    ;
constexpr float     Example_MorphTargets::mouseGrappleMomentum     ;
constexpr int       Example_MorphTargets::numBookPageKeyframes     ;
constexpr long long Example_MorphTargets::nudgePageWaitTime        ;
constexpr float     Example_MorphTargets::nudgePageSpeed           ;

LLGL_IMPLEMENT_EXAMPLE(Example_MorphTargets);



