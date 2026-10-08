/*
 * BrickPlay.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "BrickPlay.Brick.h"
#include "BrickPlay.Scene.h"
#include "BrickPlay.Utils.h"
#include "BrickPlay.UIRenderer.h"
#include "BrickPlay.UIMenu.h"
#include "BrickPlay.Const.h"
#include <ExampleBase.h>
#include <PerlinNoise.h>
#include <FileUtils.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>
#include <set>
#include <map>
#include <chrono>
#include <float.h> // for FLT_MAX

// File and string streams for simple scene file writer/reader
#include <fstream>
#include <sstream>

// Enables parallax occlusion displacement for rendering the plane studs.
#define ENABLE_PARALLAX_OCCLUSION 1


class Example_BrickPlay : public ExampleBase
{

    #define foreach_dimension(X, Y, Z, VEC) \
        for_range(Z, (VEC).z) for_range(Y, (VEC).y) for_range(X, (VEC).x)

    #if 0//LLGL_OS_WASM
    // WebGL cannot render into floating-point texture formats :-(
    static constexpr LLGL::Format   transparencyAccumFormat         = LLGL::Format::RGBA32UInt;
    #else
    // High precision format for transparency accumulation texture. Set this to either RGBA16Float or RGBA32Float.
    static constexpr LLGL::Format   transparencyAccumFormat         = LLGL::Format::RGBA16Float;
    #endif

    struct Options
    {
        std::string                 sceneFilename                   = "Scenes/Bricks.scene.txt";
        std::string                 configFilename                  = "BrickPlay.config.txt";
        bool                        discardEdits                    = false;    // Don't save edited scene
        int                         scenePresetNo                   = -1;       // Auto-generate preset scene
        int                         sceneBoundingVolume[3]          = { 100, 60, 100 };
        int                         uiMaxLeftColumns                = 2;        // Maximum number of columns on the left side of the menu
        int                         uiMaxRightColumns               = 2;        // Maximum number of columns on the right side of the menu
    }
    options;

    enum BrickPSO
    {
        BrickPSO_Default = 0,
        BrickPSO_Default_Wireframe,
        BrickPSO_Transparent,
        BrickPSO_Transparent_Wireframe,
        BrickPSO_Preview,               // With dithering to indicate blocked position

        BrickPSO_Num,
    };

    // Shaders to render brick instances, the brick preview model, UI elements, an infinite plne for the floor, and the skybox
    LLGL::Shader*                   vsBrick                         = nullptr;
    LLGL::Shader*                   fsBrick                         = nullptr;
    LLGL::Shader*                   fsBrickTransparent              = nullptr;
    LLGL::Shader*                   vsBrickPreview                  = nullptr;
    LLGL::Shader*                   fsBrickPreview                  = nullptr;
    LLGL::Shader*                   vsTransparentComposite          = nullptr;
    LLGL::Shader*                   fsTransparentComposite          = nullptr;
    LLGL::Shader*                   vsOverlay                       = nullptr;
    LLGL::Shader*                   fsOverlay                       = nullptr;
    LLGL::Shader*                   vsOutline                       = nullptr;
    LLGL::Shader*                   gsOutline                       = nullptr;
    LLGL::Shader*                   fsOutline                       = nullptr;
    LLGL::Shader*                   vsPlane                         = nullptr;
    LLGL::Shader*                   fsPlane                         = nullptr;
    LLGL::Shader*                   vsSkybox                        = nullptr;
    LLGL::Shader*                   fsSkybox                        = nullptr;

    LLGL::PipelineLayout*           psoLayout                       = nullptr;
    LLGL::PipelineState*            pso[BrickPSO_Num]               = {};

    LLGL::PipelineLayout*           psoOutlineWriteStencilLayout    = nullptr;
    LLGL::PipelineState*            psoOutlineWriteStencil          = nullptr;

    LLGL::PipelineLayout*           psoOutlineWriteColorLayout      = nullptr;
    LLGL::PipelineState*            psoOutlineWriteColor            = nullptr;

    LLGL::PipelineLayout*           psoPlaneLayout                  = nullptr;
    LLGL::PipelineState*            psoPlane[2]                     = {};

    LLGL::PipelineLayout*           psoSkyLayout                    = nullptr;
    LLGL::PipelineState*            psoSky                          = nullptr;

    LLGL::PipelineLayout*           psoCompositeLayout              = nullptr;
    LLGL::PipelineState*            psoComposite                    = nullptr;

    LLGL::Buffer*                   sceneViewCbuffer                = nullptr;
    LLGL::Buffer*                   colorPaletteCbuffer             = nullptr;
    LLGL::Buffer*                   vertexAndPreviewInstanceBuffer  = nullptr; // Encodes vertices of all meshes and the preview instance
    std::uint64_t                   previewInstanceBufferOffset     = 0;

    LLGL::Texture*                  uiMenuTexture                   = nullptr;

    LLGL::Sampler*                  uvWrapSampler                   = nullptr; // UV address wrap sampler. Anisotropic for desktop platforms.

    LLGL::Texture*                  skyboxCubemap                   = nullptr;

    bool                            hasOffsetInstancing             = false;
    bool                            hasLoadedScene                  = false;
    bool                            canCombineVertexAndIndexBuffers = false; // WebGL 2 cannot combine vertex and index buffers

    std::unique_ptr<BrickScene>     currentScene;

    Camera                          camera;
    History                         history;
    Instance                        previewInstance                 = {};

    UIRenderer                      uiRenderer;
    UILayout                        uiLayout;
    UIState                         uiState;

    struct UIBrickThumbnails
    {
        LLGL::Texture*                      arrayTexture    = nullptr;
        std::vector<LLGL::RenderTarget*>    renderTargets;

        struct Cache
        {
            GridRotation                    gridRotation    = GridRotation_Num;
            float                           viewRotation    = 0.0f;
        }
        cache;
    }
    uiBrickThumbnails;

    struct alignas(16) SceneView
    {
        Gs::Matrix4f        vpMatrix;                                               // View-projection matrix
        Gs::Matrix4f        vpMatrixInv;
        Gs::Matrix4f        vpMatrixInvNoViewPos;

        Gs::Vector3f        gridScale               = Gs::Vector3f{ 1.0f };
        float               specularPower           = 90.0f;                        // Specular exponent for blinn-phone shading model

        Gs::Vector3f        viewPos;                                                // Camera view position (in world space)
        float               skyboxDiffuseLOD        = 6.0f;

        Gs::Vector4f        ambientLight            = { 0.2f, 0.2f, 0.2f, 0.0f };
        Gs::Vector4f        diffuseLight[2]         =
        {
            Gs::Vector4f{ 0.8f, 0.8f, 0.8f, 1.0f },
            Gs::Vector4f{ 0.1f, 0.1f, 0.1f, 1.0f },
        };

        // Vector for two light sources. They point from a pixel towards the light source as it is common in shading formulas.
        Gs::Vector4f        lightDir[2]             =
        {
            Gs::Vector4f{ 0.0f,  0.0f, -1.0f, 0.0f }.Normalized(),
            Gs::Vector4f{ 0.0f, -0.2f, +1.0f, 0.0f }.Normalized()
        };

        // Preview shaders only
        float               worldOffsetY            = 0.0f;
        std::int32_t        isDithering             = 0;

        // Outline shaders only
        float               viewportSize[2]         = {};
        float               outlineColor[3]         = { Cfg::silhouetteColor[0], Cfg::silhouetteColor[1], Cfg::silhouetteColor[2] };
        float               outlineWidth            = Cfg::silhouetteWidth;

        // Plane shaders only
        std::int32_t        gridBounds[4]           = {};
        LLGL::ColorRGBAf    planeBaseColors[2]      = { LLGL::ColorRGBAf{ 1.0f }, LLGL::ColorRGBAf{ 0.5f, 0.9f, 0.5f } };
        float               parallaxFadeRange[2]    = { 75.0f, 100.0f };
        float               studHeight              = 0.225f;

        float               skyboxDiffuseFactor     = 0.2f;
    }
    sceneView;

    struct PreviewBrick
    {
        BrickId         id              = static_cast<BrickId>(0); // Previews brick with instance index 0.
        bool            isBlocked       = false;
        std::int32_t    gridOffset      = 0;
    };

    struct ViewOptions
    {
        bool            showWireframe   = false;
        bool            showMenu        = true;
        PreviewBrick    previewBrick;
    }
    viewOptions;

    struct PlaneVertex
    {
        Gs::Vector3f worldPos;
    };

    struct PlaneModel
    {
        static constexpr int            maxNumFaces             = Cfg::planeSubdivisions * Cfg::planeSubdivisions;
        static constexpr int            maxNumVertices          = maxNumFaces * 5;
        static constexpr int            maxNumIndices           = maxNumFaces * 3 * 3;
        static constexpr std::uint64_t  meshBufferIndexOffset   = sizeof(PlaneVertex) * PlaneModel::maxNumVertices;

        std::vector<PlaneVertex>    vertices;
        std::vector<std::uint16_t>  indices;

        std::uint32_t               numVertices = 0;
        std::uint32_t               numIndices  = 0;

        LLGL::Buffer*               vertexOrMeshBuffer  = nullptr; // Combine vertex with index buffer if possible
        LLGL::Buffer*               indexBuffer         = nullptr; // WebGL 2 needs index buffers separate from all other buffers
        LLGL::Texture*              colorMap            = nullptr;
        LLGL::Texture*              normalMap           = nullptr;
        LLGL::Texture*              heightMap           = nullptr;
    }
    planeModel;

    struct OpaqueFramebuffer
    {
        const LLGL::ClearValue  clearValues[1]  =
        {
            LLGL::ClearValue{}, // Default depth-stencil clear values
        };

        LLGL::Texture*          colorBuffer     = nullptr;
        LLGL::Texture*          depthStencil    = nullptr;
        LLGL::RenderTarget*     renderTarget    = nullptr;
        LLGL::RenderPass*       renderPass      = nullptr;
    }
    opacity;

    // Framebuffer resources to render transparent bricks with weighted-blended technique.
    // See https://learnopengl.com/Guest-Articles/2020/OIT/Weighted-Blended
    struct TransparencyFramebuffer
    {
        const LLGL::ClearValue  clearValues[2]  =
        {
            LLGL::ClearValue{ 0.0f, 0.0f, 0.0f, 0.0f },
            LLGL::ClearValue{ 1.0f, 0.0f, 0.0f, 0.0f },
        };

        LLGL::Texture*          accumTexture    = nullptr;
        LLGL::Texture*          revealTexture   = nullptr;
        LLGL::RenderTarget*     renderTarget    = nullptr;
        LLGL::RenderPass*       renderPass      = nullptr;
    }
    transparency;

    // Brick vertex strucutre for brick instances. They are untextured, solid-colored bricks, so they don't need texture coordinates.
    struct BrickVertex
    {
        Gs::Vector3f position;
        Gs::Vector3f normal;
    };

    std::vector<BrickVertex> brickVertices;

    // Determines what mode to use to render the highlighting of a selected scene object.
    enum class SelectionRenderMode
    {
        NativeLineWidth,    // Native line-width support to render silhouette
        EmulatedLineWidth,  // Emulated line-width support using a geometry shader (for D3D).
        Simplified,         // Simplified highlighting without a silhouette (for WebGL).
    };

    SelectionRenderMode selectionRenderMode = SelectionRenderMode::Simplified;

    // Height field and perlin noise generator parameters. See PerlinNoise::Noise().
    struct HeightFieldParameters
    {
        int             yRange[2]   = { -15, 20 };  // Y-coordinate grid range for auto-generated height fields
        std::uint32_t   frequency   = 5;
        std::uint32_t   octaves     = 5;
        float           persistence = 0.5f;
    };

    // To track what action captures mouse/touch input, in other words, the user's hands that operate the app.
    enum class HandAction
    {
        None,

        Mobile_MoveCamera,  // Navigate mobile controller on the left (camera movement)
        Mobile_TurnCamera,  // Navigate mobile controller on the right (camera rotation)

        Desktop_MoveCamera, // Camera movement with cursor. Hide cursor at start, show again at end
        Desktop_TurnCamera, // Camera rotation with cursor. Hide cursor at start, show again at end

        LeftMenu_Capture,   // Capture hand action in left menu, still has to decide whether to scroll or select color
        LeftMenu_Scroll,    // Continue left menu interaction by scrolling up/down
        LeftMenu_Select,    // Select color from left menu

        RightMenu_Capture,  // Capture hand action in right menu, still has to decide whether to scroll or drag brick out of palette
        RightMenu_Scroll,   // Continue right menu interaction by scrolling up/down

        MoveBrick,          // Moves brick that was selected from scene or pulled out of the palette from the right menu
    };

    // Just another name for user input pointer :-)
    struct UserHand
    {
        HandAction      action          = HandAction::None;
        LLGL::Offset2D  capturePos;             // Input pointer position when the capture started
        float           captureScroll   = 0.0f; // Captured menu bar scroll position
        bool            isOverMenu      = false;

        void Capture(HandAction newAction, LLGL::Offset2D pos)
        {
            action = newAction;
            capturePos = pos;
        }
    };

    // State tracking for user controls such as picking scene objects with the mouse pointer.
    struct User
    {
        User()
        {
            backupInstance.gridRotation = GridRotation_Num; // Mark as inactive backup instance with this invalid rotation value
        }

        UserHand            hand[2];

        LLGL::Offset2D      lastCursorPos;

        Brick               pickedBrick;
        int                 pickedInstanceColorIndex    = 0; // Color palette index of lasted picked brick

        Instance            backupInstance              = {};

        int                 pickedColorPaletteIndex     = 0;
        GridRotation        menuGridRotation            = GridRotation_0;
    }
    user;

    static std::uint32_t GetOverrideSwapChainSamples()
    {
        // Don't use multi-sampled swap-chain on mobile devices, it's too slow for high-resolution phones and tablets.
        return IsRunningOnMobileDevice() ? 1 : 0;
    }

public:

    Example_BrickPlay() :
        ExampleBase{ "LLGL Example: BrickPlay", Example_BrickPlay::GetOverrideSwapChainSamples() }
    {
        // Initialize kd-Tree vounding volume and camera projection parameters
        InitAppConfig();

        // Create all graphics objects
        CreateResources();
        CreateSizeDependentResources(swapChain->GetResolution());
        if (!CreatePipelines())
            return;

        // Render all bricks into thumbnail render-targets for the UI
        InitializeAllBrickThumbnails(Cfg::uiThumbnailDefaultRotation);

        // Load previoulsy saved scene or create default initial scene on failure.
        currentScene = std::unique_ptr<BrickScene>(new BrickScene(*renderer));
        if (options.scenePresetNo < 0)
        {
            if (LoadSceneFromFile(*currentScene, options.sceneFilename.c_str()))
                hasLoadedScene = true;
        }
        else
        {
            CreateDefaultScene(*currentScene, options.scenePresetNo);
            hasLoadedScene = true;
        }

        currentScene->PrintInfo();

        // Resolve initial scene collision with camera
        currentScene->ResolveCollision(camera.position, Cfg::cameraCollisionRadius);

        // Print info
        LLGL_LOG(
            "Keyboard controls:\n"
            "  1 .............. Default mode\n"
            "  2 .............. Add brick mode\n"
            "  3 .............. Move brick mode\n"
            "  4 .............. Delete brick mode\n"
            "  Right mouse .... Turn camera\n"
            "  Middle mouse ... Move camera left/right & up/down\n"
            "  W/A/S/D ........ Move camera left/right & forwards/backwards\n"
            "  PageUp/-Down ... Change brick type\n"
            "  Space .......... Rotate brick\n"
        );
    }

private:

    // Wrapper to resolve filename to full path (some platforms don't allow read directly from storage, like Android).
    bool ReadFile(const char* filename, std::stringstream& outStream)
    {
        std::string filePath;
        if (!FindAsset(filename, &filePath))
        {
            LLGL_ERROR("Failed to find file: %s\n", filename);
            return false;
        }

        LLGL::Blob fileContent = LLGL::Blob::CreateFromFile(filePath, true);
        if (!fileContent)
        {
            LLGL_ERROR("Failed to read file: %s\n", filePath.c_str());
            return false;
        }

        outStream = std::stringstream(std::string(static_cast<const char*>(fileContent.GetData())));
        return true;
    }

    // Loads the configuration file and imports its color palette and brick models
    bool ImportConfigs(const char* filename)
    {
        std::stringstream file;
        if (!ReadFile(filename, file))
            return false;

        const bool success = ParseTextFile(
            file,
            [&](std::stringstream& stream, int lineNo) -> bool
            {
                char cmd = '\0';
                stream >> cmd;

                if (cmd == 'b')
                {
                    // Parse brick model
                    std::string label;
                    stream >> label;

                    BrickInfo& brickInfo = BrickDictionary::Get().FindOrInsert(std::move(label));

                    GridVec3i brickExtent;
                    stream >> brickExtent.x >> brickExtent.y >> brickExtent.z;

                    if (brickInfo.extent != GridVec3i{} && brickInfo.extent != brickExtent)
                    {
                        LLGL_ERROR(
                            "Mismatch between initial brick '%s' extent (%d x %d x %d) and appendix group (%d x %d x %d) in %s:%d\n",
                            brickInfo.label.c_str(),
                            brickInfo.extent.x, brickInfo.extent.y, brickInfo.extent.z,
                            brickExtent.x, brickExtent.y, brickExtent.z,
                            filename, lineNo
                        );
                        return false;
                    }
                    brickInfo.extent = brickExtent;

                    // Sanity check
                    if (brickInfo.extent.x < 1 || brickInfo.extent.x > Cfg::maxBrickExtent ||
                        brickInfo.extent.y < 1 || brickInfo.extent.y > Cfg::maxBrickExtent ||
                        brickInfo.extent.z < 1 || brickInfo.extent.z > Cfg::maxBrickExtent)
                    {
                        LLGL_ERROR(
                            "Brick '%s' extent out of bounds [1, %d]: %d x %d x %d\n",
                            brickInfo.label.c_str(), Cfg::maxBrickExtent, brickInfo.extent.x, brickInfo.extent.y, brickInfo.extent.z
                        );
                        return false;
                    }

                    // Load 3D mesh and emplace into group
                    std::string modelFilename, groupName;
                    stream >> modelFilename >> groupName;

                    BrickGroup brickGroup = (groupName.empty() ? BrickGroup_Default : BrickDictionary::Get().GroupFromString(groupName.c_str()));
                    if (brickGroup >= BrickGroup_Num)
                    {
                        LLGL_ERROR("Invalid brick group '%s' in %s:%d\n", groupName.c_str(), filename, lineNo);
                        return false;
                    }
                    if (brickInfo.meshes[brickGroup].numVertices > 0)
                    {
                        LLGL_WARN(
                            "Brick '%s' overrides previous group model for '%s' in %s:%d\n",
                            brickInfo.label.c_str(), groupName.c_str(), filename, lineNo
                        );
                    }

                    // Expand upper bound of how many groups are used for this brick type
                    brickInfo.ExpandGroups(brickGroup);

                    if (!LoadBrickModelFile(brickInfo.meshes[brickGroup], modelFilename.c_str()))
                    {
                        LLGL_ERROR("Failed to load brick model '%s' in %s:%d\n", modelFilename.c_str(), filename, lineNo);
                        return false;
                    }
                }
                else if (cmd == 'e')
                {
                    // Parse excluded cell from brick model
                    std::string label;
                    stream >> label;

                    BrickInfo& brickInfo = BrickDictionary::Get().FindOrInsert(std::move(label));

                    GridVec3i excludedCell;
                    stream >> excludedCell.x >> excludedCell.y >> excludedCell.z;

                    brickInfo.excludedCells.push_back(excludedCell);
                }
                else if (cmd == 'v')
                {
                    // Parse bounding volume size
                    stream >> options.sceneBoundingVolume[0] >> options.sceneBoundingVolume[1] >> options.sceneBoundingVolume[2];
                    for (int size : options.sceneBoundingVolume)
                    {
                        if (size < 1)
                        {
                            LLGL_ERROR(
                                "Invalid scene bounding volume dimensions %dx%dx%d in %s:%d\n",
                                options.sceneBoundingVolume[0], options.sceneBoundingVolume[1], options.sceneBoundingVolume[2], filename, lineNo
                            );
                            return false;
                        }
                    }
                }
                else if (cmd == 'p')
                {
                    // Parse entry for intermediate color palette that's exclusive to this scene
                    std::string colorStr;
                    stream >> colorStr;
                    ColorPalette::Get().Insert(ColorFromString(colorStr));
                }
                else
                {
                    LLGL_ERROR("Unknown command '%c' in %s:%d\n", cmd, filename, lineNo);
                    return false;
                }

                return true;
            }
        );
        if (!success)
            return false;

        LLGL_LOG("Imported configuration file: %s\n", filename);
        return true;
    }

    // Projects the specified screen coordinate into a ray originating from the camera view to pick elements in the scene.
    Ray3f ProjectScreenPosToViewRay(LLGL::Offset2D screenPos)
    {
        const LLGL::Viewport viewport{ swapChain->GetResolution() };

        Gs::Vector4f ndc
        {
              (static_cast<float>(screenPos.x) - viewport.x) / viewport.width  * 2.0f - 1.0f,
            -((static_cast<float>(screenPos.y) - viewport.y) / viewport.height * 2.0f - 1.0f),
            1.0f,
            1.0f,
        };

        Gs::Matrix4f vpMatrixInv = projection * camera.vMatrixNoPosition;
        vpMatrixInv.MakeInverse();

        ndc = vpMatrixInv * ndc;

        return Ray3f
        {
            Gs::TransformVector(camera.wMatrix, Gs::Vector3f{}),
            Gs::Vector3f{ ndc }.Normalized()
        };
    }

    bool LoadBrickModelFile(BrickMesh& outMesh, const char* filename)
    {
        LLGL_LOG("Load brick model: %s\n", filename);

        const std::vector<TexturedVertex> intermediateVertices = Load3DModel(filename);
        if (intermediateVertices.empty())
            return false;

        outMesh.firstVertex = static_cast<std::uint32_t>(brickVertices.size());
        outMesh.numVertices = static_cast<std::uint32_t>(intermediateVertices.size());

        brickVertices.resize(brickVertices.size() + intermediateVertices.size());

        auto ConvertVertex = [](BrickVertex& dst, const TexturedVertex& src) -> void
        {
            dst.position    = src.position;
            dst.normal      = src.normal;
        };

        for (std::uint32_t i = 0; i < outMesh.numVertices; ++i)
            ConvertVertex(brickVertices[outMesh.firstVertex + i], intermediateVertices[i]);

        return true;
    }

    // If the configuration file fails to load, this is the last attempt to load some known hard-coded brick models.
    void LoadAllDefaultBrickModels()
    {
        enum ModelFlags
        {
            Model_IsWallOptimized       = (1 << 0), // Does the model has an optimized version that only renders its walls, with some potential but negligible holes?
            Model_IsWallOptimizedExt    = (1 << 1), // Does the model has an optimized version that only renders its walls, but is still 'water tight'?
            Model_IsGlassOptimized      = (1 << 2), // Does the model has a specialized version for transparent materials?
        };

        auto LoadBrickModelGroups = [this](const char* basename, const GridVec3i& extent, int flags = 0) -> void
        {
            BrickInfo& brickInfo = BrickDictionary::Get().FindOrInsert(basename);

            brickInfo.extent = extent;

            auto LoadBrickModel = [this, &brickInfo](const char* name, BrickGroup brickGroup) -> void
            {
                // Expand upper bound of how many groups are used for this brick type
                brickInfo.ExpandGroups(brickGroup);
                BrickMesh& mesh = brickInfo.meshes[brickGroup];
                const std::string filename = std::string("Bricks/") + name + ".obj";
                (void)LoadBrickModelFile(mesh, filename.c_str());
            };

            // Load base model for this group of bricks
            LoadBrickModel(basename, BrickGroup_Default);

            if ((flags & (Model_IsWallOptimized | Model_IsWallOptimizedExt)) != 0)
            {
                // Load either standard or extended model specialization that only renders its walls.
                // These are rendered when the brick is blocked above and below to improve performance by rendering less triangels and pixels.
                std::string basenameWallsOnly = std::string(basename) + "_WallsOnly";
                if (Cfg::useWaterTightBrickOptimization && (flags & Model_IsWallOptimizedExt) != 0)
                    basenameWallsOnly += "Ext";
                LoadBrickModel(basenameWallsOnly.c_str(), BrickGroup_WallsOnly);
            }

            if ((flags & Model_IsGlassOptimized) != 0)
            {
                // Load model specialization for transparent materials. These have less tubes inside them so its easier to see through them.
                const std::string basenameGlass = std::string(basename) + "_Glass";
                LoadBrickModel(basenameGlass.c_str(), BrickGroup_Transparent);
            }
        };

        LoadBrickModelGroups("Brick1x1x1", { 1,1,1 }, Model_IsWallOptimized);
        LoadBrickModelGroups("Tile1x1",    { 1,1,1 });
        LoadBrickModelGroups("Brick1x1x3", { 1,3,1 }, Model_IsWallOptimized);
        LoadBrickModelGroups("Brick1x2x1", { 1,1,2 }, Model_IsWallOptimizedExt);
        LoadBrickModelGroups("Brick1x2x3", { 1,3,2 }, Model_IsWallOptimizedExt);
        LoadBrickModelGroups("Brick2x2x1", { 2,1,2 }, Model_IsWallOptimizedExt);
        LoadBrickModelGroups("Tile2x2",    { 2,1,2 });
        LoadBrickModelGroups("Brick2x2x3", { 2,3,2 }, Model_IsWallOptimizedExt | Model_IsGlassOptimized);
        LoadBrickModelGroups("Brick4x2x1", { 4,1,2 }, Model_IsWallOptimizedExt);
        LoadBrickModelGroups("Brick4x2x3", { 4,3,2 }, Model_IsWallOptimizedExt | Model_IsGlassOptimized);
    }

    void InitSceneBoundingVolume(const LLGL::ArrayView<int> boundingVolume)
    {
        KDTreeNode::InitBoundingVolume(boundingVolume);

        // Store grid boundaries as positive minimum and negative maximum so the shader can evaluate it in a single vector operation (see fragment shader 'PSPlane')
        sceneView.gridBounds[0] = +KDTreeNode::rootAABB.bbMin.x;
        sceneView.gridBounds[1] = +KDTreeNode::rootAABB.bbMin.z;

        sceneView.gridBounds[2] = -KDTreeNode::rootAABB.bbMax.x;
        sceneView.gridBounds[3] = -KDTreeNode::rootAABB.bbMax.z;

        LLGL_LOG(
            "Scene bounding volume extent: %d x %d x %d\n",
            boundingVolume[0], boundingVolume[1], boundingVolume[2]
        );
    }

    void InitAppConfig()
    {
        // Allow user to override scene file
        if (ParseArgument("-preset", options.scenePresetNo))
            options.sceneFilename = "Scenes/Bricks.Preset" + std::to_string(options.scenePresetNo) + ".txt";

        ParseArgument("-file", options.sceneFilename);

        options.discardEdits = HasArgument("-discard");

        // Use a longer far plane for this example, since the user can pan the camera around
        projectionParams.zNear  = 1.0f;
        projectionParams.zFar   = 1000.0f;
        UpdateProjection(swapChain->GetResolution());

        // Cache feature support for offset instancing as this will be used every frame
        const LLGL::RenderingCapabilities& caps = renderer->GetRenderingCaps();
        hasOffsetInstancing = caps.features.hasOffsetInstancing;

        if (caps.limits.lineWidthRange[1] >= 2.0f)
            selectionRenderMode = SelectionRenderMode::NativeLineWidth;
        else if (caps.features.hasGeometryShaders)
            selectionRenderMode = SelectionRenderMode::EmulatedLineWidth;
        else
            selectionRenderMode = SelectionRenderMode::Simplified;

        // Import configurations and finalize color palette
        if (!ImportConfigs(options.configFilename.c_str()))
        {
            ColorPalette::Get().InitDefault();
            LoadAllDefaultBrickModels();
        }
        ColorPalette::Get().Finalize();

        // Store root AABB of kd-Tree
        std::vector<int> boundingVolumeOverride;
        if (ParseArgument("-grid", boundingVolumeOverride, 'x') && boundingVolumeOverride.size() == 3)
            InitSceneBoundingVolume(boundingVolumeOverride);
        else
            InitSceneBoundingVolume(options.sceneBoundingVolume);

        // Explicitly disable combined vertex/index buffers for WebGL backend
        canCombineVertexAndIndexBuffers = (renderer->GetRendererID() != LLGL::RendererID::WebGL);

        //TEST
        // Reduce the number of columns for the menu bars to a minum on mobile devices, due to reduced screen size.
        //if (IsRunningOnMobileDevice())
        {
            options.uiMaxLeftColumns = 1;
            options.uiMaxRightColumns = 1;
        }
    }

    LLGL::Texture* CreateDummyArrayTexture(LLGL::TextureType texType)
    {
        LLGL::TextureDescriptor cubeTexDesc;
        {
            cubeTexDesc.debugName           = "DummyArrayTexture";
            cubeTexDesc.type                = texType;
            cubeTexDesc.extent.width        = 1;
            cubeTexDesc.extent.height       = 1;
            cubeTexDesc.arrayLayers         = 6;
            cubeTexDesc.clearValue.color[0] = 1.0f;
            cubeTexDesc.clearValue.color[0] = 1.0f;
            cubeTexDesc.clearValue.color[0] = 1.0f;
            cubeTexDesc.clearValue.color[0] = 1.0f;
        }
        return renderer->CreateTexture(cubeTexDesc);
    }

    // Load textures and bakes them into a cube texture via `CopyTexture()` command.
    LLGL::Texture* LoadCubeOrArrayTexture(LLGL::TextureType texType, const std::string& basename, std::initializer_list<const char*> layerNames)
    {
        LLGL_VERIFY(texType == LLGL::TextureType::Texture2DArray || (texType == LLGL::TextureType::TextureCube && layerNames.size() == 6));

        // Load individual cube faces
        std::vector<LLGL::Texture*> layerTextures;
        layerTextures.resize(layerNames.size());

        int layerIndex = 0;
        LLGL::Extent3D layer0Extent;
        constexpr long layerBindFlags = (LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment | LLGL::BindFlags::CopySrc);

        for (const char* layerName : layerNames)
        {
            // Load cube face texture
            layerTextures[layerIndex] = LoadTexture(basename + layerName, layerBindFlags);

            // Validate all layer textures have the same size
            const LLGL::Extent3D layerExtent = layerTextures[layerIndex]->GetMipExtent(0);
            if (layerIndex > 0)
            {
                if (layerExtent != layer0Extent)
                {
                    LLGL_ERROR(
                        "Incompatible array texture sizes: layer %d (size=%ux%ux%u)\n does not match layer 0 (size=%ux%ux%u)\n",
                        layerIndex,
                        layerExtent.width, layerExtent.height, layerExtent.depth,
                        layer0Extent.width, layer0Extent.height, layer0Extent.depth
                    );
                    return CreateDummyArrayTexture(texType);
                }
            }
            else
            {
                // Also validate layer sizes are square for cube textures
                if (texType == LLGL::TextureType::TextureCube)
                {
                    if (layerExtent.width != layerExtent.height || layerExtent.depth != 1)
                    {
                        LLGL_ERROR(
                            "Incompatible cube map size: face 0 (size=%ux%ux%u) does not have a square size\n",
                            layerExtent.width, layerExtent.height, layerExtent.depth
                        );
                        return CreateDummyArrayTexture(texType);
                    }
                }
                layer0Extent = layerExtent;
            }
            ++layerIndex;
        }

        // Create array or cube texture with no initial data
        const std::uint32_t numLayers = static_cast<std::uint32_t>(layerTextures.size());

        const std::string debugName = "CubeOrArrayTexture[" + basename + "]";
        LLGL::TextureDescriptor outTexDesc;
        {
            outTexDesc.debugName    = debugName.c_str();
            outTexDesc.type         = texType;
            outTexDesc.bindFlags    = LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment | LLGL::BindFlags::CopyDst;
            outTexDesc.extent       = layer0Extent;
            outTexDesc.arrayLayers  = numLayers;
            outTexDesc.miscFlags    = LLGL::MiscFlags::NoInitialData;
        }
        LLGL::Texture* outTex = renderer->CreateTexture(outTexDesc);

        // Copy all array layers into output texture on the GPU and generate MIP-maps
        commands->Begin();
        {
            const std::string debugLabel = "BakeCubemap[" + basename + "]";
            commands->PushDebugGroup(debugLabel.c_str());
            {
                LLGL::TextureLocation dstLocation;
                const LLGL::TextureLocation srcLocation;

                for_range(layer, numLayers)
                {
                    dstLocation.arrayLayer = layer;
                    if (LLGL::Texture* layerTex = layerTextures[layer])
                        commands->CopyTexture(*outTex, dstLocation, *layerTex, srcLocation, layer0Extent);
                }

                commands->GenerateMips(*outTex);
            }
            commands->PopDebugGroup();
        }
        commands->End();
        commandQueue->Submit(*commands);

        return outTex;
    }

    // Creates all resources that depend on the swap-chain resolution.
    // This is called once at startup and continously as the window is being resized.
    void CreateSizeDependentResources(const LLGL::Extent2D& resolution)
    {
        CreateOpacityResource(resolution);
        CreateTransparencyResource(resolution);
    }

    void CreateOpacityResource(const LLGL::Extent2D& resolution)
    {
        // Delete previoulsy created resources
        if (opacity.colorBuffer != nullptr)
        {
            renderer->Release(*opacity.colorBuffer);
            renderer->Release(*opacity.depthStencil);
            renderer->Release(*opacity.renderTarget);
        }

        // Create render-target textures to render into during transparency pass
        LLGL::TextureDescriptor texDesc;
        {
            texDesc.debugName       = "Opacity.ColorBuffer";
            texDesc.extent.width    = resolution.width;
            texDesc.extent.height   = resolution.height;
            texDesc.format          = swapChain->GetColorFormat();
            texDesc.mipLevels       = 1;
            texDesc.miscFlags       = LLGL::MiscFlags::NoInitialData;
        }
        opacity.colorBuffer = renderer->CreateTexture(texDesc);
        {
            texDesc.debugName       = "Opacity.DepthStencil";
            texDesc.bindFlags       = LLGL::BindFlags::DepthStencilAttachment;
            texDesc.format          = swapChain->GetDepthStencilFormat();

            if (GetSampleCount() > 1)
            {
                texDesc.type    = LLGL::TextureType::Texture2DMS;
                texDesc.samples = GetSampleCount();
            }
        }
        opacity.depthStencil = renderer->CreateTexture(texDesc);

        // Create render-target with two attachments
        LLGL::RenderTargetDescriptor renderTargetDesc;
        {
            renderTargetDesc.debugName              = "Opacity.RenderTarget";
            renderTargetDesc.renderPass             = opacity.renderPass;
            renderTargetDesc.resolution             = resolution;
            renderTargetDesc.samples                = GetSampleCount();
            renderTargetDesc.depthStencilAttachment = opacity.depthStencil;

            if (renderTargetDesc.samples > 1)
            {
                renderTargetDesc.colorAttachments[0].format = opacity.colorBuffer->GetFormat();
                renderTargetDesc.resolveAttachments[0]      = opacity.colorBuffer;
            }
            else
            {
                renderTargetDesc.colorAttachments[0]        = opacity.colorBuffer;
            }

        }
        opacity.renderTarget = renderer->CreateRenderTarget(renderTargetDesc);
    }

    void CreateTransparencyResource(const LLGL::Extent2D& resolution)
    {
        // Delete previoulsy created resources
        if (transparency.accumTexture != nullptr)
        {
            renderer->Release(*transparency.accumTexture);
            renderer->Release(*transparency.revealTexture);
            renderer->Release(*transparency.renderTarget);
        }

        // Create render-target textures to render into during transparency pass
        LLGL::TextureDescriptor texDesc;
        {
            texDesc.debugName       = "Transparency.AccumulationTexture";
            texDesc.extent.width    = resolution.width;
            texDesc.extent.height   = resolution.height;
            texDesc.format          = transparencyAccumFormat;
            texDesc.mipLevels       = 1;
            texDesc.miscFlags       = LLGL::MiscFlags::NoInitialData;
        }
        transparency.accumTexture = renderer->CreateTexture(texDesc);
        {
            texDesc.debugName       = "Transparency.RevealTexture";
            texDesc.format          = LLGL::Format::R8UNorm;
        }
        transparency.revealTexture = renderer->CreateTexture(texDesc);

        // Create render-target with two attachments
        LLGL::RenderTargetDescriptor renderTargetDesc;
        {
            renderTargetDesc.debugName              = "Transparency.RenderTarget";
            renderTargetDesc.renderPass             = transparency.renderPass;
            renderTargetDesc.resolution             = resolution;
            renderTargetDesc.samples                = GetSampleCount();
            renderTargetDesc.depthStencilAttachment = opacity.depthStencil;

            if (renderTargetDesc.samples > 1)
            {
                renderTargetDesc.colorAttachments[0].format = transparency.accumTexture->GetFormat();
                renderTargetDesc.colorAttachments[1].format = transparency.revealTexture->GetFormat();
                renderTargetDesc.resolveAttachments[0]      = transparency.accumTexture;
                renderTargetDesc.resolveAttachments[1]      = transparency.revealTexture;
            }
            else
            {
                renderTargetDesc.colorAttachments[0]        = transparency.accumTexture;
                renderTargetDesc.colorAttachments[1]        = transparency.revealTexture;
            }
        }
        transparency.renderTarget = renderer->CreateRenderTarget(renderTargetDesc);
    }

    void CreateResources()
    {
        const float projZAxis = GetProjectionZAxis();

        // Create buffer for per-vertex data
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.debugName = "BrickVertexBuffer";
            vertexBufferDesc.size      = sizeof(BrickVertex) * brickVertices.size() + sizeof(Instance);
            vertexBufferDesc.stride    = sizeof(BrickVertex);
            vertexBufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
        }
        vertexAndPreviewInstanceBuffer  = renderer->CreateBuffer(vertexBufferDesc);
        previewInstanceBufferOffset     = sizeof(BrickVertex) * brickVertices.size();

        renderer->WriteBuffer(*vertexAndPreviewInstanceBuffer, 0, brickVertices.data(), previewInstanceBufferOffset);
        renderer->WriteBuffer(*vertexAndPreviewInstanceBuffer, previewInstanceBufferOffset, &previewInstance, sizeof(previewInstance));

        // Create scene view and color palette constant buffers
        sceneViewCbuffer = CreateConstantBuffer(sceneView);

        LLGL::BufferDescriptor colorCbufferDesc;
        {
            colorCbufferDesc.debugName  = "ColorPaletteCbuffer";
            colorCbufferDesc.size       = sizeof(LLGL::ColorRGBAub) * ColorPalette::maxNumColors;
            colorCbufferDesc.bindFlags  = LLGL::BindFlags::ConstantBuffer;
        }
        colorPaletteCbuffer = renderer->CreateBuffer(colorCbufferDesc);
        renderer->WriteBuffer(*colorPaletteCbuffer, 0, ColorPalette::Get().Data(), ColorPalette::Get().DataSize());

        // Initialize plane model
        if (canCombineVertexAndIndexBuffers)
        {
            LLGL::BufferDescriptor planeBufferDesc;
            {
                planeBufferDesc.debugName   = "PlaneMeshBuffer(Vertices+Indices)";
                planeBufferDesc.size        = sizeof(PlaneVertex) * PlaneModel::maxNumVertices + sizeof(std::uint16_t) * PlaneModel::maxNumIndices;
                planeBufferDesc.stride      = sizeof(PlaneVertex);
                planeBufferDesc.format      = LLGL::Format::R16UInt;
                planeBufferDesc.bindFlags   = LLGL::BindFlags::VertexBuffer | LLGL::BindFlags::IndexBuffer;
            }
            planeModel.vertexOrMeshBuffer = renderer->CreateBuffer(planeBufferDesc);
        }
        else
        {
            LLGL::BufferDescriptor planeVbufferDesc;
            {
                planeVbufferDesc.debugName   = "PlaneVertexBuffer";
                planeVbufferDesc.size        = sizeof(PlaneVertex) * PlaneModel::maxNumVertices;
                planeVbufferDesc.stride      = sizeof(PlaneVertex);
                planeVbufferDesc.bindFlags   = LLGL::BindFlags::VertexBuffer;
            }
            planeModel.vertexOrMeshBuffer = renderer->CreateBuffer(planeVbufferDesc);

            LLGL::BufferDescriptor planeIbufferDesc;
            {
                planeIbufferDesc.debugName   = "PlaneIndexBuffer";
                planeIbufferDesc.size        = sizeof(std::uint16_t) * PlaneModel::maxNumIndices;
                planeIbufferDesc.format      = LLGL::Format::R16UInt;
                planeIbufferDesc.bindFlags   = LLGL::BindFlags::IndexBuffer;
            }
            planeModel.indexBuffer = renderer->CreateBuffer(planeIbufferDesc);
        }

        planeModel.vertices.resize(PlaneModel::maxNumVertices);
        planeModel.indices.resize(PlaneModel::maxNumIndices);

        planeModel.colorMap     = LoadTexture("Bricks/BrickHead_Albedo.png");
        planeModel.normalMap    = LoadTexture("Bricks/BrickHead_Normals.png");
        planeModel.heightMap    = LoadTexture("Bricks/BrickHead_Heights.png", LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment, LLGL::Format::R8UNorm);

        // Create primary texture sampler
        LLGL::SamplerDescriptor samplerDesc;
        {
            // Only use anisotropic filtering on desktop platforms.
            // This is quite expensive on high-resolution mobile devices, so fall back to standard tri-linear filtering.
            if (!IsRunningOnMobileDevice())
                samplerDesc.maxAnisotropy = 8;
        }
        uvWrapSampler = renderer->CreateSampler(samplerDesc);

        // Load skybox textures
        skyboxCubemap = LoadCubeOrArrayTexture(
            LLGL::TextureType::TextureCube, "Skybox/sky_74_cubemap_2k/",
            { "px.png", "nx.png", "py.png", "ny.png", "pz.png", "nz.png" }
        );

        // Load all UI element textures. For high-resolution displays, load the larger 64px UI elements.
        LLGL::Display* primaryDisplay = LLGL::Display::GetPrimary();
        if (primaryDisplay != nullptr && primaryDisplay->GetScale() >= 2.0f)
        {
            uiMenuTexture = LoadCubeOrArrayTexture(
                LLGL::TextureType::Texture2DArray, "UIElements/",
                { "download64.png", "upload64.png", "arrow64.png", "rotate64.png", "add64.png", "select64.png", "trash64.png" }
            );
        }
        else
        {
            uiMenuTexture = LoadCubeOrArrayTexture(
                LLGL::TextureType::Texture2DArray, "UIElements/",
                { "download32.png", "upload32.png", "arrow32.png", "rotate32.png", "add32.png", "select32.png", "trash32.png" }
            );
        }

        LLGL_VERIFY(uiMenuTexture != nullptr);
        const LLGL::Extent3D uiMenuTextureExtent = uiMenuTexture->GetMipExtent(0);
        InitUILayoutForElementSize(static_cast<int>(uiMenuTextureExtent.width));

        // Create render pass for opacity scene rendering.
        LLGL::RenderPassDescriptor opacityPassDesc;
        {
            opacityPassDesc.debugName                   = "Opacity.RenderPass";
            opacityPassDesc.samples                     = GetSampleCount();

            opacityPassDesc.colorAttachments[0].format  = swapChain->GetColorFormat();
            opacityPassDesc.colorAttachments[0].loadOp  = LLGL::AttachmentLoadOp::Undefined;
            opacityPassDesc.colorAttachments[0].storeOp = LLGL::AttachmentStoreOp::Store;

            opacityPassDesc.depthAttachment.format      = swapChain->GetDepthStencilFormat();
            opacityPassDesc.depthAttachment.loadOp      = LLGL::AttachmentLoadOp::Clear;
            opacityPassDesc.depthAttachment.storeOp     = LLGL::AttachmentStoreOp::Store;
        }
        opacity.renderPass = renderer->CreateRenderPass(opacityPassDesc);

        // Create render pass for transparenty scene rendering.
        LLGL::RenderPassDescriptor transparencyPassDesc;
        {
            transparencyPassDesc.debugName                      = "Transparency.RenderPass";
            transparencyPassDesc.samples                        = GetSampleCount();

            transparencyPassDesc.colorAttachments[0].format     = transparencyAccumFormat;
            transparencyPassDesc.colorAttachments[0].loadOp     = LLGL::AttachmentLoadOp::Clear;
            transparencyPassDesc.colorAttachments[0].storeOp    = LLGL::AttachmentStoreOp::Store;

            transparencyPassDesc.colorAttachments[1].format     = LLGL::Format::R8UNorm;
            transparencyPassDesc.colorAttachments[1].loadOp     = LLGL::AttachmentLoadOp::Clear;
            transparencyPassDesc.colorAttachments[1].storeOp    = LLGL::AttachmentStoreOp::Store;

            // Only load depth-stencil attachment, but don't write to it
            transparencyPassDesc.depthAttachment.format         = swapChain->GetDepthStencilFormat();
            transparencyPassDesc.depthAttachment.loadOp         = LLGL::AttachmentLoadOp::Load;
            transparencyPassDesc.depthAttachment.storeOp        = LLGL::AttachmentStoreOp::Undefined;
        }
        transparency.renderPass = renderer->CreateRenderPass(transparencyPassDesc);
    }

    bool CreatePipelines()
    {
        #define REPORT_PSO_ERRORS(PSO)      \
            do                              \
            {                               \
                if (ReportPSOErrors(PSO))   \
                    return false;           \
            }                               \
            while (false)

        const LLGL::ShaderMacro fsPlaneMacros[]
        {
            LLGL::ShaderMacro{ "ENABLE_PARALLAX_OCCLUSION", ENABLE_PARALLAX_OCCLUSION ? "1" : "0" },
            LLGL::ShaderMacro{},
        };

        const LLGL::ShaderMacro fsTransparentMacros[]
        {
            LLGL::ShaderMacro{ "ENABLE_DUAL_SIDED_LIGHTING", "1" },
            LLGL::ShaderMacro{},
        };

        // Load shaders. Apply clipping-origin patch to ESSL since glClipControl() is not supported in WebGL.
        vsBrick                 = LoadVertexShader  ("BrickPlay", "VSBrick", nullptr, LLGL::ShaderCompileFlags::PatchClippingOrigin);
        fsBrick                 = LoadFragmentShader("BrickPlay", "PSBrick");
        fsBrickTransparent      = LoadFragmentShader("BrickPlay", "PSBrickTransparent", fsTransparentMacros);
        vsBrickPreview          = LoadVertexShader  ("BrickPlay", "VSBrickPreview", nullptr, LLGL::ShaderCompileFlags::PatchClippingOrigin);
        fsBrickPreview          = LoadFragmentShader("BrickPlay", "PSBrickPreview");
        vsTransparentComposite  = LoadVertexShader  ("BrickPlay", "VSTransparentComposite");
        fsTransparentComposite  = LoadFragmentShader("BrickPlay", "PSTransparentComposite");
        vsOverlay               = LoadVertexShader  ("BrickPlay", "VSOverlay");
        fsOverlay               = LoadFragmentShader("BrickPlay", "PSOverlay");
        vsPlane                 = LoadVertexShader  ("BrickPlay", "VSPlane", nullptr, LLGL::ShaderCompileFlags::PatchClippingOrigin);
        fsPlane                 = LoadFragmentShader("BrickPlay", "PSPlane", fsPlaneMacros);
        vsSkybox                = LoadVertexShader  ("BrickPlay", "VSSkybox", nullptr, LLGL::ShaderCompileFlags::PatchClippingOrigin);
        fsSkybox                = LoadFragmentShader("BrickPlay", "PSSkybox");

        // Initialize UI renderer
        if (!uiRenderer.Initialize(renderer.get(), *swapChain, vsOverlay, fsOverlay))
        {
            Quit(1);
            return false;
        }

        // Create pipeline layout for instanced scene rendering
        psoLayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneView@2):vert:frag,"
                "cbuffer(ColorPalette@3):vert,"

                "texture(skyboxTexture@7):frag,"
                "sampler(skyboxSampler@8){}:frag,"
                "sampler<skyboxTexture, skyboxSampler>(s_skyboxTextureskyboxSampler@7),"
            )
        );

        // Specify vertex formats
        const LLGL::VertexAttribute vertexAttribs[] =
        {
            // Per-vertex attributes
            LLGL::VertexAttribute{ "position",  LLGL::Format::RGB32Float, /*location:*/ 0, offsetof(BrickVertex, position), sizeof(BrickVertex), /*slot:*/ 0 },
            LLGL::VertexAttribute{ "normal",    LLGL::Format::RGB32Float, /*location:*/ 1, offsetof(BrickVertex, normal  ), sizeof(BrickVertex), /*slot:*/ 0 },

            // Per-instance attributes
            LLGL::VertexAttribute{ "packedInstance", LLGL::Format::RGBA16SInt, /*location:*/ 2, 0, sizeof(Instance), /*slot:*/ 1, /*instanceDivisor:*/ 1 },
        };

        // Create graphics PSO for default scene rendering
        auto SetPSODefaults = [this, &vertexAttribs](LLGL::GraphicsPipelineDescriptor& psoDesc) -> void
        {
            psoDesc.renderPass                      = opacity.renderPass;
            psoDesc.inputVertexAttribs              = vertexAttribs;
            psoDesc.pipelineLayout                  = psoLayout;
            psoDesc.primitiveTopology               = LLGL::PrimitiveTopology::TriangleList;
            psoDesc.depth.testEnabled               = true;
            psoDesc.depth.writeEnabled              = true;
            psoDesc.rasterizer.multiSampleEnabled   = (GetSampleCount() > 1);
            psoDesc.rasterizer.cullMode             = LLGL::CullMode::Back;
        };

        LLGL::GraphicsPipelineDescriptor psoDesc;
        {
            SetPSODefaults(psoDesc);
            psoDesc.debugName               = "Scene.Opaque.PSO";
            psoDesc.vertexShader            = vsBrick;
            psoDesc.fragmentShader          = fsBrick;
        }
        pso[BrickPSO_Default] = renderer->CreatePipelineState(psoDesc);
        REPORT_PSO_ERRORS(pso[BrickPSO_Default]);
        {
            psoDesc.debugName               = "Scene.Opaque.Wireframe.PSO";
            psoDesc.rasterizer.polygonMode  = LLGL::PolygonMode::Wireframe;
        }
        pso[BrickPSO_Default_Wireframe] = renderer->CreatePipelineState(psoDesc);
        REPORT_PSO_ERRORS(pso[BrickPSO_Default_Wireframe]);

        // Create PSO for transparent bricks
        LLGL::GraphicsPipelineDescriptor psoTransparentDesc;
        {
            SetPSODefaults(psoTransparentDesc);

            psoTransparentDesc.debugName                        = "Scene.Transparent.PSO";
            psoTransparentDesc.renderPass                       = transparency.renderPass;
            psoTransparentDesc.vertexShader                     = vsBrick;
            psoTransparentDesc.fragmentShader                   = fsBrickTransparent;

            psoTransparentDesc.depth.writeEnabled               = false;
            psoTransparentDesc.blend.independentBlendEnabled    = true;

            psoTransparentDesc.blend.targets[0].blendEnabled    = true;
            psoTransparentDesc.blend.targets[0].srcColor        = LLGL::BlendOp::One;
            psoTransparentDesc.blend.targets[0].srcAlpha        = LLGL::BlendOp::One;
            psoTransparentDesc.blend.targets[0].dstColor        = LLGL::BlendOp::One;
            psoTransparentDesc.blend.targets[0].dstAlpha        = LLGL::BlendOp::One;

            psoTransparentDesc.blend.targets[1].blendEnabled    = true;
            psoTransparentDesc.blend.targets[1].srcColor        = LLGL::BlendOp::Zero;
            psoTransparentDesc.blend.targets[1].srcAlpha        = LLGL::BlendOp::Zero;
            psoTransparentDesc.blend.targets[1].dstColor        = LLGL::BlendOp::InvSrcColor;
            psoTransparentDesc.blend.targets[1].dstAlpha        = LLGL::BlendOp::Zero;
            psoTransparentDesc.blend.targets[1].colorMask       = LLGL::ColorMaskFlags::R;
        }
        pso[BrickPSO_Transparent] = renderer->CreatePipelineState(psoTransparentDesc);
        REPORT_PSO_ERRORS(pso[BrickPSO_Transparent]);
        {
            psoTransparentDesc.debugName                        = "Scene.Transparent.Wireframe.PSO";
            psoTransparentDesc.rasterizer.polygonMode           = LLGL::PolygonMode::Wireframe;
        }
        pso[BrickPSO_Transparent_Wireframe] = renderer->CreatePipelineState(psoTransparentDesc);
        REPORT_PSO_ERRORS(pso[BrickPSO_Transparent_Wireframe]);

        // Create PSO for blocked preview object
        LLGL::GraphicsPipelineDescriptor psoPreviewDesc;
        {
            SetPSODefaults(psoPreviewDesc);
            psoPreviewDesc.debugName                            = "Scene.Preview.PSO";
            psoPreviewDesc.vertexShader                         = vsBrickPreview;
            psoPreviewDesc.fragmentShader                       = fsBrickPreview;
            psoPreviewDesc.rasterizer.polygonMode               = LLGL::PolygonMode::Fill;
            psoPreviewDesc.rasterizer.depthBias.constantFactor  = -100.0f;
        }
        pso[BrickPSO_Preview] = renderer->CreatePipelineState(psoPreviewDesc);
        REPORT_PSO_ERRORS(pso[BrickPSO_Preview]);

        if (selectionRenderMode != SelectionRenderMode::Simplified)
        {
            // Create PSO to render silhouette outline when selecting bricks
            psoOutlineWriteStencilLayout = renderer->CreatePipelineLayout(LLGL::Parse("cbuffer(SceneView@2):vert"));

            vsOutline = LoadVertexShader  ("BrickPlay", "VSBrickOutline");
            fsOutline = LoadFragmentShader("BrickPlay", "PSBrickOutline");

            // Create PSO to write stencil to mask out pixels that are inside the silhouette
            LLGL::GraphicsPipelineDescriptor psoOutlineDesc;
            {
                psoOutlineDesc.debugName                        = "EmulatedLineWidthPSO-WriteStencil";
                psoOutlineDesc.pipelineLayout                   = psoOutlineWriteStencilLayout;
                psoOutlineDesc.inputVertexAttribs               = vertexAttribs;
                psoOutlineDesc.renderPass                       = swapChain->GetRenderPass();

                // This PSO only renders a stencil value, so no fragment shader is provided
                psoOutlineDesc.vertexShader                     = vsOutline;

                // Enable stencil test and write out a reference value of 1
                psoOutlineDesc.stencil.testEnabled              = true;
                psoOutlineDesc.stencil.front.depthPassOp        = LLGL::StencilOp::Replace;
                psoOutlineDesc.stencil.front.compareOp          = LLGL::CompareOp::AlwaysPass;
                psoOutlineDesc.stencil.front.readMask           = 0x0;
                psoOutlineDesc.stencil.front.writeMask          = 0x1;
                psoOutlineDesc.stencil.front.reference          = 0x1;

                psoOutlineDesc.rasterizer.cullMode              = LLGL::CullMode::Back;
                psoOutlineDesc.rasterizer.multiSampleEnabled    = (swapChain->GetSamples() > 1);
                psoOutlineDesc.blend.targets[0].colorMask       = LLGL::ColorMaskFlags::Zero;
            }
            psoOutlineWriteStencil = renderer->CreatePipelineState(psoOutlineDesc);
            REPORT_PSO_ERRORS(psoOutlineWriteStencil);

            // Create PSO to read stencil and write color output for silhouette
            if (selectionRenderMode == SelectionRenderMode::NativeLineWidth)
            {
                // Use native support for line width
                psoOutlineWriteColorLayout = renderer->CreatePipelineLayout(LLGL::Parse("cbuffer(SceneView@2):vert:frag"));

                psoOutlineDesc.pipelineLayout                   = psoOutlineWriteColorLayout;
                psoOutlineDesc.rasterizer.lineWidth             = Cfg::silhouetteWidth;
                psoOutlineDesc.rasterizer.polygonMode           = LLGL::PolygonMode::Wireframe;
            }
            else
            {
                // Emulate line width using a geometry shader
                gsOutline = LoadGeometryShader("BrickPlay", "GSBrickOutline");

                psoOutlineWriteColorLayout = renderer->CreatePipelineLayout(LLGL::Parse("cbuffer(SceneView@2):vert:geom:frag"));

                psoOutlineDesc.pipelineLayout                   = psoOutlineWriteColorLayout;
                psoOutlineDesc.geometryShader                   = gsOutline;
            }

            {
                psoOutlineDesc.debugName                        = "EmulatedLineWidthPSO-WriteColor";
                psoOutlineDesc.fragmentShader                   = fsOutline;

                // Enable stencil test and read a reference value of 1, in which case no pixels are written to mask out pixels inside the silhouette
                psoOutlineDesc.stencil.testEnabled              = true;
                psoOutlineDesc.stencil.front.depthPassOp        = LLGL::StencilOp::Keep;
                psoOutlineDesc.stencil.front.compareOp          = LLGL::CompareOp::NotEqual;
                psoOutlineDesc.stencil.front.readMask           = 0x1;
                psoOutlineDesc.stencil.front.writeMask          = 0x0;
                psoOutlineDesc.stencil.front.reference          = 0x1;

                psoOutlineDesc.blend.targets[0].colorMask       = LLGL::ColorMaskFlags::All;
            }
            psoOutlineWriteColor = renderer->CreatePipelineState(psoOutlineDesc);
            REPORT_PSO_ERRORS(psoOutlineWriteColor);
        }

        // Create PSO for plane rendering
        psoPlaneLayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneView@2):vert:frag,"
                "sampler(planeSampler@3):frag,"
                "texture(planeColorMap@4):frag,"
                "texture(planeNormalMap@5):frag,"

                "texture(skyboxTexture@7):frag,"
                "sampler(skyboxSampler@8){}:frag,"

                #if ENABLE_PARALLAX_OCCLUSION
                "texture(planeHeightMap@6):frag,"
                "sampler<planeHeightMap, planeSampler>(s_planeHeightMapplaneSampler@6),"
                #endif

                "sampler<planeColorMap, planeSampler>(s_planeColorMapplaneSampler@4),"
                "sampler<planeNormalMap, planeSampler>(s_planeNormalMapplaneSampler@5),"
                "sampler<skyboxTexture, skyboxSampler>(s_skyboxTextureskyboxSampler@7),"
            )
        );

        const LLGL::VertexAttribute planeVertexAttribs[] =
        {
            LLGL::VertexAttribute{ "worldPos", LLGL::Format::RGB32Float, /*location:*/ 0, /*offset:*/ offsetof(PlaneVertex, worldPos), /*stride:*/ sizeof(PlaneVertex) },
        };

        LLGL::GraphicsPipelineDescriptor psoPlaneDesc;
        {
            psoPlaneDesc.debugName                      = "Plane.PSO";
            psoPlaneDesc.renderPass                     = opacity.renderPass;
            psoPlaneDesc.inputVertexAttribs             = planeVertexAttribs;
            psoPlaneDesc.vertexShader                   = vsPlane;
            psoPlaneDesc.fragmentShader                 = fsPlane;
            psoPlaneDesc.pipelineLayout                 = psoPlaneLayout;
            psoPlaneDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleList;
            psoPlaneDesc.depth.testEnabled              = true;
            psoPlaneDesc.depth.writeEnabled             = true;
            psoPlaneDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        };
        psoPlane[0] = renderer->CreatePipelineState(psoPlaneDesc);
        REPORT_PSO_ERRORS(psoPlane[0]);
        {
            psoPlaneDesc.rasterizer.polygonMode         = LLGL::PolygonMode::Wireframe;
        }
        psoPlane[1] = renderer->CreatePipelineState(psoPlaneDesc);
        REPORT_PSO_ERRORS(psoPlane[1]);

        // Create PSO for skybox rendering
        psoSkyLayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneView@2):vert,"
                "texture(skyboxTexture@7):frag,"
                "sampler(skyboxSampler@8){}:frag," // Default static sampler

                "sampler<skyboxTexture, skyboxSampler>(s_skyboxTextureskyboxSampler@7),"
            )
        );

        LLGL::GraphicsPipelineDescriptor psoSkyDesc;
        {
            psoSkyDesc.debugName                        = "Skybox.PSO";
            psoSkyDesc.renderPass                       = opacity.renderPass;
            psoSkyDesc.vertexShader                     = vsSkybox;
            psoSkyDesc.fragmentShader                   = fsSkybox;
            psoSkyDesc.pipelineLayout                   = psoSkyLayout;
            psoSkyDesc.primitiveTopology                = LLGL::PrimitiveTopology::TriangleList;
            psoSkyDesc.depth.testEnabled                = true;
            psoSkyDesc.depth.compareOp                  = LLGL::CompareOp::LessEqual;
            psoSkyDesc.rasterizer.multiSampleEnabled    = (GetSampleCount() > 1);
        };
        psoSky = renderer->CreatePipelineState(psoSkyDesc);
        REPORT_PSO_ERRORS(psoSky);

        // Create transparency composition PSO
        psoCompositeLayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "texture(colorTexture@2):frag,"
                "texture(accumTexture@3):frag,"
                "texture(revealTexture@4):frag,"
            )
        );

        LLGL::GraphicsPipelineDescriptor psoCompositeDesc;
        {
            psoCompositeDesc.debugName                      = "Transparency.Composite.PSO";
            psoCompositeDesc.pipelineLayout                 = psoCompositeLayout;
            psoCompositeDesc.renderPass                     = swapChain->GetRenderPass();
            psoCompositeDesc.vertexShader                   = vsTransparentComposite;
            psoCompositeDesc.fragmentShader                 = fsTransparentComposite;
            psoCompositeDesc.rasterizer.multiSampleEnabled  = (swapChain->GetSamples() > 1);
        }
        psoComposite = renderer->CreatePipelineState(psoCompositeDesc);
        REPORT_PSO_ERRORS(psoComposite);

        return true;
    }

    bool SaveSceneToStream(BrickScene& scene, std::ostream& file)
    {
        // Write header
        file << "# Scene file for LLGL's example 'BrickPlay'" << std::endl;
        file << "# Current brick type: 'b <DIMENSIONS>' e.g. 'b 2x2x3', supported dimensions are 2x2x3, 4x2x3, 4x2x1" << std::endl;
        file << "# New brick instance: 'i X Y Z ROTATE RGBA' e.g. 'i 10 0 -3 90 FFFF00FF', ROTATE must be 0 or 90" << std::endl;
        file << "# Camera posiotion: 'c X Y Z PITCH YAW' e.g. 'i 10.2 5.7 -15.02 25.2 -15.1'" << std::endl;
        file << "# Current offset: 'o X Y Z' e.g. 'o 10 0 0' moves all subsequent bricks +10 on the X-axis" << std::endl;

        // Write camera position
        file
            << 'c'
            << ' ' << camera.position.x
            << ' ' << camera.position.y
            << ' ' << camera.position.z
            << ' ' << camera.pitch
            << ' ' << camera.yaw
            << std::endl;

        // Write intermediate color palette if number of colors are less than 50% of bricks. Otherwise, write colors individually for each brick
        std::map<std::uint32_t, int> colorToPaletteIndex;

        std::size_t numTotalInstances = 0;
        scene.ForEachInstanceGroup(
            [&](BrickId brickId, BrickGroup groupType, const InstanceGroup& group)
            {
                // Write new brick type
                for_range(i, group.numInstances)
                {
                    const Instance& instance = scene.GetInstanceByIndex(group.firstInstance + i);
                    colorToPaletteIndex.insert(std::make_pair(ColorPalette::Get().IndexToPacked(instance.colorIndex), 0));
                }
                numTotalInstances += group.numInstances;
            },
            /*includeAllGroups:*/ true
        );

        const bool useColorPalette = (colorToPaletteIndex.size() < numTotalInstances/2);
        if (useColorPalette)
        {
            // Write color for palette and assign their indices
            int colorPaletteIndex = 0;
            for (auto& it : colorToPaletteIndex)
            {
                file << "p " << ColorToString(Uint32ToColor(it.first)) << std::endl;
                it.second = colorPaletteIndex++;
            }
        }

        // Write all instances for each brick type
        BrickId lastWrittenBrickType = INVALID_BRICK_ID;
        char colorStr[9] = {};

        auto ColorToStringOrPaletteIndex = [&](LLGL::ColorRGBAub color) -> const char*
        {
            // Try to find color in palette first
            if (useColorPalette)
            {
                auto it = colorToPaletteIndex.find(ColorToUint32(color));
                if (it != colorToPaletteIndex.end())
                {
                    ::snprintf(colorStr, sizeof(colorStr), "p%d", it->second);
                    return colorStr;
                }
            }

            // Otherwise, write individual color
            return ColorToString(color);
        };

        scene.ForEachInstanceGroup(
            [&](BrickId brickId, BrickGroup groupType, const InstanceGroup& group)
            {
                // Write new brick type
                if (group.numInstances > 0)
                {
                    if (lastWrittenBrickType != brickId)
                    {
                        lastWrittenBrickType = brickId;
                        file << "b " << BrickDictionary::Get().TypeToString(brickId) << std::endl;
                    }
                }

                // Write all instances
                for_range(i, group.numInstances)
                {
                    const Instance& instance = scene.GetInstanceByIndex(group.firstInstance + i);
                    file
                        << 'i'
                        << ' ' << instance.gridPos[0]
                        << ' ' << instance.gridPos[1]
                        << ' ' << instance.gridPos[2]
                        << ' ' << static_cast<int>(instance.gridRotation * 90)
                        << ' ' << ColorToStringOrPaletteIndex(ColorPalette::Get().IndexToColor(instance.colorIndex))
                        << std::endl;
                }
            },
            /*includeAllGroups:*/ true
        );

        // Mark file as saved after scaing completed successful
        scene.MarkSaved();

        return true;
    }

    bool SaveSceneToFile(BrickScene& scene, const char* filename)
    {
        // Open file for writing
        std::ofstream file{ filename };
        if (!file.good())
        {
            LLGL_ERROR("Failed to save scene to file: %s\n", filename);
            return false;
        }
        if (SaveSceneToStream(scene, file))
        {
            LLGL_LOG("Saved scene to file: %s\n", filename);
            return true;
        }
        return false;
    }

    bool LoadSceneFromFile(BrickScene& scene, const char* filename)
    {
        std::stringstream file;
        if (!ReadFile(filename, file))
            return false;

        BrickId currentBrickType = INVALID_BRICK_ID;
        GridVec3i currentOffset;

        std::size_t numSkippedBricks = 0;
        std::size_t numBlockedBricks = 0;

        std::vector<LLGL::ColorRGBAub> intermediateColors;

        auto ColorOrPaletteIndexFromString = [&intermediateColors](const std::string& s) -> LLGL::ColorRGBAub
        {
            // Try to parse palette index first
            if (!s.empty() && s.front() == 'p')
            {
                int paletteIndex = std::atoi(&s[1]);
                if (paletteIndex < intermediateColors.size())
                    return intermediateColors[paletteIndex];

                LLGL_ERROR("Color palette index %d out of bounds\n", paletteIndex);
                return {};
            }

            // Otherwise, parse individual color value
            return ColorFromString(s);
        };

        // Parse every line and interpret it
        const bool success = ParseTextFile(
            file,
            [&](std::stringstream& stream, int lineNo) -> bool
            {
                // Parse current line with stringstream
                char cmd = '\0';
                stream >> cmd;

                if (cmd == 'i')
                {
                    if (currentBrickType == INVALID_BRICK_ID)
                    {
                        ++numSkippedBricks;
                        return true;
                    }

                    // Parse instance data
                    GridVec3i gridPos;
                    int gridRotation = 0;
                    std::string colorStr;

                    stream >> gridPos.x >> gridPos.y >> gridPos.z >> gridRotation >> colorStr;

                    if (gridRotation % 90 != 0)
                        LLGL_ERROR("Ignored unknown brick rotation %d (must be a multiple of 90) in %s:%d\n", gridRotation, filename, lineNo);

                    // Find closest color from global palette
                    int colorIndex = ColorPalette::Get().FindClosestColorIndex(ColorOrPaletteIndexFromString(colorStr));

                    // Create new brick instance
                    if (!scene.NewBrick(currentBrickType, gridPos + currentOffset, static_cast<GridRotation>(gridRotation / 90), colorIndex))
                    {
                        // Count how many bricks were blocked to report it as warning
                        ++numBlockedBricks;
                    }
                }
                else if (cmd == 'b')
                {
                    // Parse brick type and make it current
                    std::string brickTypeStr;
                    stream >> brickTypeStr;
                    currentBrickType = BrickDictionary::Get().TypeFromString(brickTypeStr.c_str());

                    if (currentBrickType == INVALID_BRICK_ID)
                    {
                        currentBrickType = BrickDictionary::Get().FindClosestTypeFromString(brickTypeStr.c_str());
                        if (currentBrickType != INVALID_BRICK_ID)
                        {
                            const std::string& altBrickLabel = BrickDictionary::Get().GetInfo(currentBrickType).label;
                            LLGL_WARN(
                                "Unknown brick type '%s' in %s:%d; Found alternative '%s'\n",
                                brickTypeStr.c_str(), filename, lineNo, altBrickLabel.c_str()
                            );
                        }
                        else
                            LLGL_ERROR("Unknown brick type '%s' in %s:%d\n", brickTypeStr.c_str(), filename, lineNo);
                    }
                }
                else if (cmd == 'o')
                {
                    // Parse grid offset
                    stream >> currentOffset.x >> currentOffset.y >> currentOffset.z;
                }
                else if (cmd == 'c')
                {
                    // Parse camera orientation
                    stream >> camera.position.x >> camera.position.y >> camera.position.z >> camera.pitch >> camera.yaw;
                }
                else if (cmd == 'p')
                {
                    if (currentBrickType != INVALID_BRICK_ID)
                    {
                        LLGL_ERROR("Color palette entries must appear before brick types\n");
                        scene.ResetScene();
                        return false;
                    }

                    // Parse entry for intermediate color palette that's exclusive to this scene
                    std::string colorStr;
                    stream >> colorStr;
                    intermediateColors.push_back(ColorFromString(colorStr));
                }
                else
                {
                    LLGL_ERROR("Unknown command '%c' in %s:%d\n", cmd, filename, lineNo);
                    scene.ResetScene();
                    return false;
                }

                return true;
            }
        );
        if (!success)
            return false;

        // Mark scene as saved after loading, since no changes have been made that are not saved in the file we just loaded.
        scene.MarkSaved();

        LLGL_LOG("Loaded scene from file: %s\n", filename);

        if (numSkippedBricks > 0 || numBlockedBricks > 0)
        {
            LLGL_WARN(
                "Missing bricks: %zu skipped, %zu blocked by other bricks\n",
                numSkippedBricks, numBlockedBricks
            );
        }

        return true;
    }

    void BuildBrickChunkFromHeightField(
        BrickScene& scene,
        int xOffset, int zOffset, int xExtent, int zExtent,
        const int (&yRange)[2], const std::vector<float>& heightField, int rowStride,
        BrickId brickId1x1, BrickId brickId1x1Tile)
    {
        // Gather height field values and min/max range
        float fieldMin = FLT_MAX;
        float fieldMax = 0.0f;
        float field[Cfg::heightFieldChunkSize][Cfg::heightFieldChunkSize];

        for (int z = 0; z < zExtent; ++z)
        {
            for (int x = 0; x < xExtent; ++x)
            {
                field[z][x] = heightField[(zOffset + z) * rowStride + (xOffset + x)];
                fieldMin = std::min(fieldMin, field[z][x]);
                fieldMax = std::max(fieldMax, field[z][x]);
            }
        }

        const int levelColorIndices[3]
        {
            ColorPalette::Get().FindClosestColorIndex(LLGL::ColorRGBAub{ 255, 255, 255 }),
            ColorPalette::Get().FindClosestColorIndex(LLGL::ColorRGBAub{ 200, 255, 190 }),
            ColorPalette::Get().FindClosestColorIndex(LLGL::ColorRGBAub{  90, 220, 100 }),
        };

        auto BrickPainter = [&levelColorIndices](BrickId brickId, const GridVec3i& gridPos) -> int
        {
            int level = gridPos.y/4;
            return levelColorIndices[Gs::Clamp(level, 0, 2)];
        };

        // Build foundation with largest bricks possible
        int heightFieldUpperBound = (yRange[1] - yRange[0]);
        int yMaxChunk = static_cast<int>(fieldMin * heightFieldUpperBound);

        const GridVec3i bbOffset
        {
            KDTreeNode::rootAABB.bbMin.x + xOffset,
            KDTreeNode::rootAABB.bbMin.y + yRange[0],
            KDTreeNode::rootAABB.bbMin.z + zOffset
        };
        scene.NewBricksChunk(bbOffset, GridVec3i{ xExtent, yMaxChunk, zExtent }, BrickPainter);

        // Build finer details at the top of the current chunk
        const float fieldHeightRange = (fieldMax - fieldMax);

        for (int z = 0; z < zExtent; ++z)
        {
            for (int x = 0; x < xExtent; ++x)
            {
                int yField = static_cast<int>(field[z][x] * heightFieldUpperBound);
                if (yField > yMaxChunk)
                {
                    const BrickId brickId = (yField > yMaxChunk + 1 ? brickId1x1 : brickId1x1Tile);
                    if (brickId != INVALID_BRICK_ID)
                    {
                        const GridVec3i gridPos
                        {
                            KDTreeNode::rootAABB.bbMin.x + xOffset + x,
                            KDTreeNode::rootAABB.bbMin.y + yRange[0] + yMaxChunk,
                            KDTreeNode::rootAABB.bbMin.z + zOffset + z
                        };
                        scene.NewBrick(brickId, gridPos, GridRotation_0, BrickPainter(brickId, gridPos));
                    }
                }
            }
        }
    }

    void CreateDefaultScene(BrickScene& scene, int presetNo)
    {
        // Measure time to generate height field
        auto startTime = std::chrono::system_clock::now();
        {
            const HeightFieldParameters fieldParams[]
            {
                HeightFieldParameters{ { -15, 20 }, 5, 5, 0.50f }, // Smooth hills
                HeightFieldParameters{ { -35, 35 }, 2, 5, 0.35f }, // A lonely hill
                HeightFieldParameters{ { -15, 20 }, 4, 4, 1.00f }, // Sharp spikes in the hills, looks noiser
            };

            constexpr int numPresets = (sizeof(fieldParams)/sizeof(fieldParams[0]));
            GenerateHeightFieldScene(scene, fieldParams[presetNo % numPresets]);
        }
        auto endTime = std::chrono::system_clock::now();
        auto elapsedSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

        LLGL_LOG("Generated height field scene in %.03fs\n", static_cast<double>(elapsedSeconds) / 1000.0);
    }

    void GenerateHeightFieldScene(BrickScene& scene, const HeightFieldParameters& heightFieldParams)
    {
        // Obtain brick IDs for 1x1 and 1x1_tile bricks
        const BrickId brickId1x1 = BrickDictionary::Get().TypeFromString("1x1x1");
        const BrickId brickId1x1Tile = BrickDictionary::Get().TypeFromString("1x1_tile");

        // Get height field dimensions
        const Gs::Vector3f  bbCenter = (KDTreeNode::rootAABB.bbMax + KDTreeNode::rootAABB.bbMin).Cast<float>() * 0.5f;
        const GridVec3i     bbExtent = (KDTreeNode::rootAABB.bbMax - KDTreeNode::rootAABB.bbMin);

        // Generate height field from Perlin noise
        std::vector<float> noiseBuffer;
        PerlinNoise{}.GenerateBuffer(noiseBuffer, bbExtent.x, 1, bbExtent.z, heightFieldParams.frequency, heightFieldParams.octaves, heightFieldParams.persistence);

        for (int z = 0; z + Cfg::heightFieldChunkSize < bbExtent.z; z += Cfg::heightFieldChunkSize)
        {
            for (int x = 0; x + Cfg::heightFieldChunkSize < bbExtent.x; x += Cfg::heightFieldChunkSize)
            {
                int yRange[2] = { (heightFieldParams.yRange[0] / 3) * 3, heightFieldParams.yRange[1] };
                BuildBrickChunkFromHeightField(scene, x, z, Cfg::heightFieldChunkSize, Cfg::heightFieldChunkSize, yRange, noiseBuffer, bbExtent.x, brickId1x1, brickId1x1Tile);
            }
        }

        // Place initial camera position
        camera.position = { bbCenter.x, bbCenter.y + static_cast<float>(bbExtent.y)*0.5f, bbCenter.z - static_cast<float>(bbExtent.z)*0.7f };

        const Gs::Vector3f lookAtVector = camera.position - Gs::Vector3f{ bbCenter.x, 0.0f, bbCenter.z };
        camera.pitch    = 180.0f - Gs::Rad2Deg(Gs::Angle(lookAtVector, Gs::Vector3f{ 0, 0, 1 }));
    }

    //////////////////// RENDERING ////////////////////

    void BindInstancePSO(BrickPSO psoType, LLGL::Buffer* instanceBuffer, LLGL::VertexBufferView (&vertexBufferViews)[2])
    {
        // Set vertex buffers (vertices and instances)
        vertexBufferViews[0] = LLGL::VertexBufferView{ vertexAndPreviewInstanceBuffer };
        vertexBufferViews[1] = LLGL::VertexBufferView{ instanceBuffer };

        // If offset instancing is not supported, set vertex buffers with an offset for each instance group
        if (hasOffsetInstancing)
            commands->SetVertexBuffers(2, vertexBufferViews);

        // Bind PSO and resources
        commands->SetPipelineState(*pso[psoType]);
        commands->SetResource(0, *sceneViewCbuffer);
        commands->SetResource(1, *colorPaletteCbuffer);
        commands->SetResource(2, *skyboxCubemap);
    }

    void RenderBrickInstanceGroup(const BrickMesh mesh, const InstanceGroup& group, LLGL::VertexBufferView (&vertexBufferViews)[2])
    {
        if (hasOffsetInstancing)
        {
            // Draw all group instances
            commands->DrawInstanced(mesh.numVertices, mesh.firstVertex, group.numInstances, group.firstInstance);
        }
        else
        {
            // Bind vertex buffers with instance offset if not natively supported
            vertexBufferViews[1].offset = group.firstInstance * sizeof(Instance);
            commands->SetVertexBuffers(2, vertexBufferViews);
            commands->DrawInstanced(mesh.numVertices, mesh.firstVertex, group.numInstances);
        }
    }

    void RenderAllBricksForGroup(const BrickScene& scene, BrickGroup groupType, LLGL::VertexBufferView (&vertexBufferViews)[2])
    {
        for_range(brickId, BrickDictionary::Get().NumBrickTypes())
        {
            const BrickMesh mesh = GetBrickMeshOrDefault(brickId, groupType);
            const InstanceGroup& group = scene.GetInstanceGroup(brickId, groupType);
            if (group.numInstances > 0 && mesh.numVertices > 0)
            {
                commands->PushDebugGroup(group.debugName.c_str());
                {
                    RenderBrickInstanceGroup(mesh, group, vertexBufferViews);
                }
                commands->PopDebugGroup();
            }
        }
    }

    void RenderOpaqueBrickInstances(const BrickScene& scene)
    {
        // Exit early if no instances have been allocated yet
        LLGL::Buffer* instanceBuffer = scene.GetInstanceBuffer();
        if (instanceBuffer == nullptr)
            return;

        // Set vertex buffers (vertices and instances)
        LLGL::VertexBufferView vertexBufferViews[2];
        BindInstancePSO(viewOptions.showWireframe ? BrickPSO_Default_Wireframe : BrickPSO_Default, instanceBuffer, vertexBufferViews);

        // Draw instances of all groups
        for (int groupType = BrickGroup_Default; groupType < BrickGroup_Transparent; ++groupType)
            RenderAllBricksForGroup(scene, static_cast<BrickGroup>(groupType), vertexBufferViews);
    }

    void RenderTransparentBrickInstances(const BrickScene& scene)
    {
        // Exit early if no instances have been allocated yet
        LLGL::Buffer* instanceBuffer = scene.GetInstanceBuffer();
        if (instanceBuffer == nullptr)
            return;

        // Set vertex buffers (vertices and instances)
        LLGL::VertexBufferView vertexBufferViews[2];
        BindInstancePSO(viewOptions.showWireframe ? BrickPSO_Transparent_Wireframe : BrickPSO_Transparent, instanceBuffer, vertexBufferViews);

        // Draw instances of the transparent group only
        const BrickGroup groupType = BrickGroup_Transparent;
        RenderAllBricksForGroup(scene, static_cast<BrickGroup>(groupType), vertexBufferViews);
    }

    // Combines opaque with transparent geometry
    void RenderTransparentComposite()
    {
        // The transparency composition is rendered with a fullscreen triangle, i.e. the vertex shader generates the vertices on its own. No vertex buffer needed.
        commands->SetPipelineState(*psoComposite);

        commands->SetResource(0, *opacity.colorBuffer);
        commands->SetResource(1, *transparency.accumTexture);
        commands->SetResource(2, *transparency.revealTexture);

        commands->Draw(3, 0);
    }

    void UpdatePlaneGeometryForViewFrustum()
    {
        // Reset plane model state
        planeModel.numVertices = 0;
        planeModel.numIndices = 0;

        // Transform vertices into frustum for all subdivision segments
        const Gs::Matrix4f frustumMatrix = (projection * camera.vMatrix).Inverse();

        // NDC-to-world unprojection requires a perspective divide by W; a plain Vector4->Vector3 cast only truncates .xyz
        auto UnprojectNDC = [&frustumMatrix](float x, float y, float z) -> Gs::Vector3f
        {
            const Gs::Vector4f clip = frustumMatrix * Gs::Vector4f{ x, y, z, 1.0f };
            return Gs::Vector3f{ clip.x, clip.y, clip.z } * (1.0f / clip.w);
        };

        const Gs::Vector3f frustumLeftTop     = UnprojectNDC(-1.0f, +1.0f, +1.0f);
        const Gs::Vector3f frustumRightTop    = UnprojectNDC(+1.0f, +1.0f, +1.0f);
        const Gs::Vector3f frustumRightBottom = UnprojectNDC(+1.0f, -1.0f, +1.0f);
        const Gs::Vector3f frustumLeftBottom  = UnprojectNDC(-1.0f, -1.0f, +1.0f);

        // Eye is the camera's world position; no need to unproject the clip-space origin for this
        const Gs::Vector3f eye = camera.position;

        static_assert(Cfg::planeSubdivisions > 0, "`planeSubdivisions` must be greater than 0");
        static_assert(Cfg::planeSubdivisions <= 16, "`planeSubdivisions` should not be too large; consider keeping it at or below 16!");

        Gs::Vector3f segmentedVertices[Cfg::planeSubdivisions + 1][Cfg::planeSubdivisions + 1];

        const float planeSubdivisionsInv = 1.0f / static_cast<float>(Cfg::planeSubdivisions);
        for (int x = 0; x <= Cfg::planeSubdivisions; ++x)
        {
            float u = static_cast<float>(x) * planeSubdivisionsInv;

            const Gs::Vector3f horizontalTop    = Gs::Lerp(frustumLeftTop,    frustumRightTop,    u);
            const Gs::Vector3f horizontalBottom = Gs::Lerp(frustumLeftBottom, frustumRightBottom, u);

            for (int y = 0; y <= Cfg::planeSubdivisions; ++y)
            {
                float v = static_cast<float>(y) * planeSubdivisionsInv;

                segmentedVertices[x][y] = Gs::Lerp(horizontalTop, horizontalBottom, v);
            }
        }

        auto GetPlaneIntersection = [](const Gs::Vector3f& eye, const Gs::Vector3f& vert) -> Gs::Vector3f
        {
            float interpolator = 0.0f;
            const Ray3f ray{ eye, (vert - eye).Normalized() };

            const Gs::Vector3f planeNormal{ 0, 1, 0 };
            if (IntersectRayWithPlane(ray, planeNormal, 0.0f, &interpolator))
                return ray.Interpolate(interpolator);
            else
                return eye;
        };

        // Now generate plane vertices by projecting them into the scene
        for (int x = 0; x < Cfg::planeSubdivisions; ++x)
        {
            for (int y = 0; y < Cfg::planeSubdivisions; ++y)
            {
                Gs::Vector3f inVertices[4], outVertices[5];

                inVertices[0] = segmentedVertices[x    ][y    ];
                inVertices[1] = segmentedVertices[x + 1][y    ];
                inVertices[2] = segmentedVertices[x + 1][y + 1];
                inVertices[3] = segmentedVertices[x    ][y + 1];

                int numOutVertices = 0;
                for (int i = 0; i < 4; ++i)
                {
                    const Gs::Vector3f& vert = inVertices[i];
                    const Gs::Vector3f& prevVert = inVertices[(i - 1) & 3];

                    if (vert.y > 0.0f)
                    {
                        if (prevVert.y <= 0.0f)
                        {
                            float t = prevVert.y / (prevVert.y - vert.y);
                            outVertices[numOutVertices++] = Gs::Lerp(prevVert, vert, t);
                        }
                    }
                    else
                    {
                        if (prevVert.y > 0.0f)
                        {
                            float t = prevVert.y / (prevVert.y - vert.y);
                            outVertices[numOutVertices++] = Gs::Lerp(prevVert, vert, t);
                        }

                        // Add intersection between eye-to-vertex with plane
                        outVertices[numOutVertices++] = GetPlaneIntersection(eye, vert);
                    }
                }

                // Too many or not enough output vertices -> skip segment
                if (numOutVertices < 3 || numOutVertices > 5)
                    continue;

                // Write output vertices
                for (int i = 0; i < numOutVertices; ++i)
                {
                    PlaneVertex& planeVert = planeModel.vertices[planeModel.numVertices + i];
                    planeVert.worldPos = { outVertices[i].x, 0.0f, outVertices[i].z };
                }

                // Write output indices
                for (int i = 0; i + 2 < numOutVertices; ++i)
                {
                    planeModel.indices[planeModel.numIndices + i*3    ] = planeModel.numVertices;
                    planeModel.indices[planeModel.numIndices + i*3 + 1] = planeModel.numVertices + i + 1;
                    planeModel.indices[planeModel.numIndices + i*3 + 2] = planeModel.numVertices + i + 2;
                }

                planeModel.numVertices += numOutVertices;
                planeModel.numIndices += (numOutVertices - 2)*3;
            }
        }

        // Upload both vertex and index data to the GPU
        if (planeModel.numVertices > 0 && planeModel.numIndices > 0)
        {
            // Update plane vertex data
            commands->UpdateBuffer(*planeModel.vertexOrMeshBuffer, 0, planeModel.vertices.data(), sizeof(PlaneVertex) * planeModel.numVertices);

            // Update plane index data
            if (canCombineVertexAndIndexBuffers)
                commands->UpdateBuffer(*planeModel.vertexOrMeshBuffer, PlaneModel::meshBufferIndexOffset, planeModel.indices.data(), sizeof(std::uint16_t) * planeModel.numIndices);
            else
                commands->UpdateBuffer(*planeModel.indexBuffer, 0, planeModel.indices.data(), sizeof(std::uint16_t) * planeModel.numIndices);
        }
    }

    void RenderPlane()
    {
        commands->SetPipelineState(*psoPlane[viewOptions.showWireframe ? 1 : 0]);
        commands->SetResource(0, *sceneViewCbuffer);
        commands->SetResource(1, *uvWrapSampler);
        commands->SetResource(2, *planeModel.colorMap);
        commands->SetResource(3, *planeModel.normalMap);

        commands->SetResource(4, *skyboxCubemap);

        #if ENABLE_PARALLAX_OCCLUSION
        commands->SetResource(5, *planeModel.heightMap);
        #endif

        if (canCombineVertexAndIndexBuffers)
            commands->SetIndexBuffer(*planeModel.vertexOrMeshBuffer, LLGL::Format::R16UInt, PlaneModel::meshBufferIndexOffset);
        else
            commands->SetIndexBuffer(*planeModel.indexBuffer);

        commands->SetVertexBuffer(*planeModel.vertexOrMeshBuffer);

        commands->DrawIndexed(planeModel.numIndices, 0);
    }

    void RenderSkybox()
    {
        // Skybopx is rendered with a fullscreen triangle, i.e. the vertex shader generates the vertices on its own. No vertex buffer needed.
        commands->SetPipelineState(*psoSky);

        commands->SetResource(0, *sceneViewCbuffer);
        commands->SetResource(1, *skyboxCubemap);

        commands->Draw(3, 0);
    }

    void RenderBrickScene(const BrickScene& scene)
    {
        commands->PushDebugGroup("RenderOpaqueBrickInstances");
        {
            RenderOpaqueBrickInstances(scene);
        }
        commands->PopDebugGroup();

        if (planeModel.numIndices > 0)
        {
            commands->PushDebugGroup("RenderPlane");
            {
                RenderPlane();
            }
            commands->PopDebugGroup();
        }

        commands->PushDebugGroup("RenderSkybox");
        {
            RenderSkybox();
        }
        commands->PopDebugGroup();
    }

    void BindPreviewBrickPSOStates()
    {
        commands->SetPipelineState(*pso[BrickPSO_Preview]);
        commands->SetResource(0, *sceneViewCbuffer);
        commands->SetResource(1, *colorPaletteCbuffer);
        commands->SetResource(2, *skyboxCubemap);

        // Set vertex buffers (vertices and instances).
        // We don't care about offset instancing here, because the preview instance is always the first one (index 0).
        LLGL::VertexBufferView vertexBufferViews[] =
        {
            LLGL::VertexBufferView{ vertexAndPreviewInstanceBuffer },
            LLGL::VertexBufferView{ vertexAndPreviewInstanceBuffer, sizeof(Instance), previewInstanceBufferOffset },
        };
        commands->SetVertexBuffers(2, vertexBufferViews);
    }

    // Returns the mesh information for the specified brick type with a preferred group.
    // If the preferred group is not available for the brick type, it falls back to the default group.
    BrickMesh GetBrickMeshOrDefault(BrickId brickId, BrickGroup preferredGroup = BrickGroup_Default)
    {
        if (preferredGroup != BrickGroup_Default)
        {
            const BrickMesh preferredMesh = BrickDictionary::Get().GetMesh(brickId, preferredGroup);
            if (preferredMesh.numVertices > 0)
                return preferredMesh;
        }
        return BrickDictionary::Get().GetMesh(brickId);
    }

    void RenderPreviewBrick(const PreviewBrick& previewBrick)
    {
        const bool isPreviewBrickTransparent = ColorPalette::Get().IsTransparent(previewInstance.colorIndex);
        const BrickMesh mesh = GetBrickMeshOrDefault(previewBrick.id, isPreviewBrickTransparent ? BrickGroup_Transparent : BrickGroup_Default);
        if (mesh.numVertices > 0)
            commands->Draw(mesh.numVertices, mesh.firstVertex);
    }

    void RenderSilhouette(const BrickScene& scene, Brick brick)
    {
        // Exit early if no instances have been allocated yet
        LLGL::Buffer* instanceBuffer = scene.GetInstanceBuffer();
        if (instanceBuffer == nullptr)
            return;

        const Instance* brickInstance = scene.GetBrickInstance(brick);
        if (brickInstance == nullptr)
            return;

        const BrickMesh visibleMesh = BrickDictionary::Get().GetMesh(brick.GetId());
        if (visibleMesh.numVertices == 0)
            return;

        const InstanceGroup& pickedGroup = scene.GetInstanceGroupById(brick.groupId);

        // Bind vertex buffers for single instance. No need to rely on offset instancing support, just always use vertex buffer offset.
        LLGL::VertexBufferView vertexBufferViews[] =
        {
            LLGL::VertexBufferView{ vertexAndPreviewInstanceBuffer },
            LLGL::VertexBufferView{ instanceBuffer, 0, (pickedGroup.firstInstance + brick.groupInstance) * sizeof(Instance) },
        };
        commands->SetVertexBuffers(2, vertexBufferViews);

        // Clear stencil buffer for silhouette rendering
        commands->Clear(LLGL::ClearFlags::Stencil);

        // First render stencil reference to mask out pixels inside the silhouette. This is the regular brick model.
        commands->PushDebugGroup("RenderSilhouetteStencil");
        {
            LLGL_VERIFY(psoOutlineWriteStencil != nullptr);
            commands->SetPipelineState(*psoOutlineWriteStencil);
            commands->SetResource(0, *sceneViewCbuffer);
            commands->Draw(visibleMesh.numVertices, visibleMesh.firstVertex);
        }
        commands->PopDebugGroup();

        // Now render silhouette by rendering with a wireframe model with thick lines
        commands->PushDebugGroup("RenderSilhouetteColor");
        {
            LLGL_VERIFY(psoOutlineWriteColor != nullptr);
            commands->SetPipelineState(*psoOutlineWriteColor);
            commands->SetResource(0, *sceneViewCbuffer);
            commands->Draw(visibleMesh.numVertices, visibleMesh.firstVertex);
        }
        commands->PopDebugGroup();
    }

    void TransformSceneViewForThumbnail(BrickId brickId, GridRotation gridRotation, float viewRotation, SceneView& outView)
    {
        const float dist = 25.0f;

        const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brickId, gridRotation);
        const GridVec3i brickCenter = (brickDim.offset*2 + brickDim.extent);
        const GridVec3i unitOffset{ brickCenter.x % 2, brickDim.extent.y, brickCenter.z % 2 };

        Gs::Matrix4f vMatrix;
        Gs::Translate(vMatrix, unitOffset.Cast<float>() * KDTreeNode::gridScale * 0.5f);
        Gs::RotateY(vMatrix, Gs::Deg2Rad(viewRotation));
        Gs::RotateX(vMatrix, Gs::Deg2Rad(45.0f));
        Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -dist });

        outView.viewPos     = Gs::TransformVector(vMatrix, Gs::Vector3f{});
        outView.gridScale   = KDTreeNode::gridScale;

        // Visually, we only want to rotate the brick, but they only support 90 degree angles.
        // So instead, we turn the camera around it but still want the light to look like it shines straight against the brick.
        outView.lightDir[0] = -Gs::TransformVector(vMatrix, Gs::Vector4f{ 0.0f, -0.25f, +1.0f, 0.0f }).Normalized();

        vMatrix.MakeInverse();

        // Use orthogonal projection for brick thumbnails
        const Gs::Vector3f brickExtentScaled = brickDim.extent.Cast<float>() * KDTreeNode::gridScale;
        const float maxBrickExtent = std::max(brickExtentScaled.x, std::max(brickExtentScaled.y, brickExtentScaled.z));
        const float volumeSize = std::max(Cfg::uiThumbnailMinVolumeSize, maxBrickExtent + 1.0f);

        const Gs::Matrix4f thumbnailProjection = OrthogonalProjection(volumeSize, volumeSize, 0.1f, dist*2.0f);
        outView.vpMatrix = thumbnailProjection * vMatrix;
    }

    void RenderBrickThumnail(LLGL::RenderTarget& renderTarget, BrickId brickId, GridRotation gridRotation, float viewRotation)
    {
        SceneView thumbnailSceneView;

        // Move camera so that the thumbnail for the current brick type is clearly visible
        TransformSceneViewForThumbnail(brickId, gridRotation, viewRotation, thumbnailSceneView);
        commands->UpdateBuffer(*sceneViewCbuffer, 0, &thumbnailSceneView, sizeof(thumbnailSceneView));

        commands->BeginRenderPass(renderTarget);
        {
            const std::uint32_t thumbnailSize = static_cast<std::uint32_t>(uiLayout.brickThumbnailSize);
            commands->SetViewport(LLGL::Extent2D{ thumbnailSize, thumbnailSize });
            commands->Clear(LLGL::ClearFlags::ColorDepth, LLGL::ClearValue{});

            // Draw current thumbnail brick from default group
            const BrickMesh mesh = BrickDictionary::Get().GetMesh(brickId);
            if (mesh.numVertices > 0)
                commands->Draw(mesh.numVertices, mesh.firstVertex);
        }
        commands->EndRenderPass();
    }

    void CreateAllBrickThumbnails()
    {
        const std::uint32_t numBrickTypes = BrickDictionary::Get().NumBrickTypes();

        // In case porperties of thumbnails must be updated (such as size), delete previous texture
        if (uiBrickThumbnails.arrayTexture != nullptr)
        {
            renderer->Release(*uiBrickThumbnails.arrayTexture);
            uiBrickThumbnails.arrayTexture = nullptr;

            for (LLGL::RenderTarget* renderTarget : uiBrickThumbnails.renderTargets)
                renderer->Release(*renderTarget);
        }

        // Create thumbnail array texture
        const std::uint32_t thumbnailSize = static_cast<std::uint32_t>(uiLayout.brickThumbnailSize);
        LLGL::TextureDescriptor texArrayDesc;
        {
            texArrayDesc.debugName      = "BrickThumbnail.Texture2DArray";
            texArrayDesc.type           = LLGL::TextureType::Texture2DArray;
            texArrayDesc.extent         = { thumbnailSize, thumbnailSize, 1 };
            texArrayDesc.arrayLayers    = numBrickTypes;                    // Each brick type gets its own slot in the array texture
            texArrayDesc.mipLevels      = 1;                                // No MIP-maps since this is used for UI elements
            texArrayDesc.miscFlags      = LLGL::MiscFlags::NoInitialData;   // Don't initialize the texture since we render into it right below
        }
        uiBrickThumbnails.arrayTexture = renderer->CreateTexture(texArrayDesc);

        // Create intermediate render-targets, one for each array layer
        uiBrickThumbnails.renderTargets.resize(numBrickTypes);

        for_range(brickId, numBrickTypes)
        {
            const std::string rtLabel = std::string("RenderTarget.Thumbnail_") + BrickDictionary::Get().TypeToString(brickId);
            LLGL::RenderTargetDescriptor rtDesc;
            {
                rtDesc.debugName                        = rtLabel.c_str();
                rtDesc.resolution                       = { thumbnailSize, thumbnailSize };
                rtDesc.samples                          = GetSampleCount();
                rtDesc.depthStencilAttachment.format    = swapChain->GetDepthStencilFormat();   // Use same depth format for thumbnails as for swap-chain (to make PSOs happy)

                // Render into array layer designated for current brick type
                if (rtDesc.samples > 1)
                {
                    rtDesc.colorAttachments[0].format       = LLGL::Format::RGBA8UNorm;
                    rtDesc.resolveAttachments[0].texture    = uiBrickThumbnails.arrayTexture;
                    rtDesc.resolveAttachments[0].arrayLayer = brickId;
                }
                else
                {
                    rtDesc.colorAttachments[0].texture      = uiBrickThumbnails.arrayTexture;
                    rtDesc.colorAttachments[0].arrayLayer   = brickId;
                }
            }
            uiBrickThumbnails.renderTargets[brickId] = renderer->CreateRenderTarget(rtDesc);
        }
    }

    void RenderAllBrickThumbnails(GridRotation thumbnailRotation, float viewRotation = 45.0f)
    {
        // Reset preview instance to default
        Instance thumbnailInstance = {};
        thumbnailInstance.gridRotation = thumbnailRotation;

        // Render bricks as thumbnails into all offscreen render-targets
        commands->UpdateBuffer(*vertexAndPreviewInstanceBuffer, previewInstanceBufferOffset, &thumbnailInstance, sizeof(thumbnailInstance));

        BindPreviewBrickPSOStates();

        commands->PushDebugGroup("RenderBrickThumbnails");
        {
            for_range(brickId, BrickDictionary::Get().NumBrickTypes())
            {
                const std::string debugLabel = std::string("RenderThumbnail_") + BrickDictionary::Get().TypeToString(brickId);
                commands->PushDebugGroup(debugLabel.c_str());
                {
                    RenderBrickThumnail(*uiBrickThumbnails.renderTargets[brickId], brickId, thumbnailRotation, viewRotation);
                }
                commands->PopDebugGroup();
            }
        }
        commands->PopDebugGroup();
    }

    void RenderAllBrickThumbnailsIfChanged(GridRotation thumbnailRotation, float viewRotation = 45.0f)
    {
        if (uiBrickThumbnails.cache.gridRotation != thumbnailRotation ||
            uiBrickThumbnails.cache.viewRotation != viewRotation)
        {
            RenderAllBrickThumbnails(thumbnailRotation, viewRotation);
            uiBrickThumbnails.cache.gridRotation = thumbnailRotation;
            uiBrickThumbnails.cache.viewRotation = viewRotation;
        }
    }

    void InitializeAllBrickThumbnails(GridRotation thumbnailRotation)
    {
        CreateAllBrickThumbnails();

        commands->Begin();
        {
            RenderAllBrickThumbnails(thumbnailRotation);
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

    unsigned NumInputPointers() const
    {
        // This app recognizes up to 2 touch inputs on mobile. Only 1 mouse input for desktop.
        return IsRunningOnMobileDevice() ? 2 : 1;
    }

    unsigned FindInputPointerInRect(int x, int y, int w, int h, bool canSelect = true, bool ignoreMainMenu = false) const
    {
        auto IsPointInRect = [&](LLGL::Offset2D point) -> bool
        {
            return (point.x >= x && point.x <= x + w && point.y >= y && point.y <= y + h);
        };

        auto IsInputPointerActiveInRect = [&](unsigned pointerIndex) -> bool
        {
            const LLGL::InputPointer& inputPointer = input.GetPointer(pointerIndex);
            return
            (
                canSelect && inputPointer.state != LLGL::InputState::Inactive &&
                (!ignoreMainMenu || inputPointer.position.y > uiLayout.mainMenuSize) &&
                IsPointInRect(inputPointer.position)
            );
        };

        if (canSelect)
        {
            for_range(pointerIndex, NumInputPointers())
            {
                if (IsInputPointerActiveInRect(pointerIndex))
                    return pointerIndex + 1;
            }
        }
        return 0;
    }

    unsigned MapInputPointerSelection(unsigned pointerId)
    {
        return pointerId > 0 && IsInputUp(input.GetPointer(pointerId - 1)) ? pointerId : 0;
    }

    unsigned DrawUIColorPicker(int x, int y, int paletteIndex, bool canSelect, bool isHighlighted = false)
    {
        const int colorPickerSize = uiLayout.colorPickerSize;
        int w = colorPickerSize + UILayout::buttonFrameSize*2;
        int h = colorPickerSize + UILayout::buttonFrameSize*2;
        const unsigned pointerId = FindInputPointerInRect(x, y, w, h, canSelect, true);

        LLGL::ColorRGBAub color = ColorPalette::Get()[paletteIndex];
        const UIRenderMode render = (color.a < 255 ? UIRenderMode_Dithered : UIRenderMode_Solid);

        if (pointerId != 0 || isHighlighted)
        {
            // Only draw color button when selected
            uiRenderer.Draw2DRect(x, y, w, h, color, render);
        }
        else
        {
            // Draw background frame and then the foreground color
            uiRenderer.Draw2DRect(x, y, w, h, uiLayout.buttonFrameColor);
            uiRenderer.Draw2DRect(x + UILayout::buttonFrameSize, y + UILayout::buttonFrameSize, colorPickerSize, colorPickerSize, color, render);
        }

        return MapInputPointerSelection(pointerId);
    }

    bool DrawUIBrickThumbnail(int x, int y, int brickType, bool canSelect, bool isHighlighted = false)
    {
        const int thumbnailSize = uiLayout.brickThumbnailSize;
        int w = thumbnailSize + UILayout::buttonFrameSize*2;
        int h = thumbnailSize + UILayout::buttonFrameSize*2;
        const unsigned pointerId = FindInputPointerInRect(x, y, w, h, canSelect, true);

        const LLGL::ColorRGBAub brickColor = ColorPalette::Get()[user.pickedColorPaletteIndex];

        if (pointerId != 0 || isHighlighted)
        {
            // Draw button background to highlight selection
            uiRenderer.Draw2DRect(x, y, w, h, Cfg::uiBrickThumbnailBgColor);
        }

        // Draw thumbnail texture layer
        uiRenderer.Draw2DImage(x + UILayout::buttonFrameSize, y + UILayout::buttonFrameSize, uiBrickThumbnails.arrayTexture, brickType, brickColor);

        return (pointerId != 0);
    }

    bool DrawUIMenuItem(UIMenuItem menuItem, bool canSelect, bool isHighlighted = false, bool isDisabled = false)
    {
        int w = uiLayout.mainMenuItemSize + UILayout::buttonFrameSize*2;
        int h = uiLayout.mainMenuItemSize + UILayout::buttonFrameSize*2;
        const unsigned pointerId = (isDisabled ? 0 : FindInputPointerInRect(uiState.x, uiState.y, w, h, canSelect));

        if (pointerId != 0 || isHighlighted)
        {
            // Draw button background to highlight selection
            uiRenderer.Draw2DRect(uiState.x, uiState.y, w, h, Cfg::uiBrickThumbnailBgColor);
        }

        // Draw menu icon
        const UIMenuInfo& menuInfo = uiLayout.menuInfos[menuItem];
        uiRenderer.Draw2DImage(uiState.x + UILayout::buttonFrameSize, uiState.y + UILayout::buttonFrameSize, uiMenuTexture, menuInfo.layer, LLGL::ColorRGBAub{}, menuInfo.flags);

        if (isDisabled)
        {
            // Draw dithered grid over the icon to visualize that it's disabled
            uiRenderer.Draw2DRect(uiState.x, uiState.y, w, h, Cfg::uiBrickThumbnailBgColor, UIRenderMode_Dithered);
        }

        // Advance menu item position
        uiState.x += uiState.advance;
        if (menuInfo.endsWithSeparator)
            uiState.x += uiLayout.mainMenuSeparatorSize;

        return (MapInputPointerSelection(pointerId) != 0);
    }

    void InitUILayoutForElementSize(int uiElementSize)
    {
        uiLayout.InitForElementSize(uiElementSize);

        // Cache width of main menu
        uiState.mainMenuWidth = 0;
        for_range(menuItem, UIMenuItem_Num)
        {
            uiState.mainMenuWidth += uiLayout.mainMenuBaseAdvance;
            if (uiLayout.menuInfos[menuItem].endsWithSeparator)
                uiState.mainMenuWidth += uiLayout.mainMenuSeparatorSize;
        }
    }

    void UpdateUILayout(float dt)
    {
        // Determine number of menu columns before drawing them, to allow scrolling
        uiState.UpdateScreenSize(swapChain->GetResolution());

        uiState.ResetToMenuBar(uiLayout, uiState.menuBarLeft, uiLayout.colorPickerSize);
        {
            for_range(i, ColorPalette::Get().Count())
            {
                uiState.AdvanceColumn(uiLayout);
                if (uiState.columns > options.uiMaxLeftColumns)
                    break;
                uiState.AdvanceRow();
            }
        }
        uiState.FinalizeMenuBar(uiLayout, ColorPalette::Get().Count(), options.uiMaxLeftColumns);

        uiState.ResetToMenuBar(uiLayout, uiState.menuBarRight, uiLayout.brickThumbnailSize);
        {
            for_range(brickId, BrickDictionary::Get().NumBrickTypes())
            {
                uiState.AdvanceColumn(uiLayout);
                if (uiState.columns > options.uiMaxRightColumns)
                    break;
                uiState.AdvanceRow();
            }
        }
        uiState.FinalizeMenuBar(uiLayout, BrickDictionary::Get().NumBrickTypes(), options.uiMaxRightColumns);

        // Update scrolling
        const float scrollDir = static_cast<float>(-input.GetWheelMotion()) * Cfg::uiScrollSpeed;
        if (unsigned pointerIndex = FindInputPointerInRect(0, uiLayout.mainMenuSize, uiState.menuBarLeft.width, uiState.scrH - uiLayout.mainMenuSize))
        {
            uiState.menuBarLeft.scroll.AddForces(scrollDir);
        }
        uiState.menuBarLeft.UpdateScrolling(dt, uiState.scrH);

        if (unsigned pointerIndex = FindInputPointerInRect(uiState.scrW - uiState.menuBarRight.width, uiLayout.mainMenuSize, uiState.menuBarRight.width, uiState.scrH - uiLayout.mainMenuSize))
        {
            uiState.menuBarRight.scroll.AddForces(scrollDir);
        }
        uiState.menuBarRight.UpdateScrolling(dt, uiState.scrH);
    }

    void DrawScrollIndicator(const UIState::MenuBar& menuBar)
    {
        const int xOffset = (menuBar.isRightAligned ? uiState.scrW - menuBar.width : 0);
        const int yOffset = uiLayout.mainMenuSize;

        const int yScroll = static_cast<int>(menuBar.scroll.position);
        const int yScrollMax = menuBar.height - uiState.scrH;

        const LLGL::ColorRGBAub fadeColor{ uiLayout.mainMenuColor[0].ToRGB(), 0 };

        if (yScroll > 0)
        {
            uiRenderer.Draw2DRectGradient(
                xOffset, yOffset, menuBar.width, Cfg::uiScrollIndicatorSize,
                uiLayout.mainMenuColor[1], fadeColor
            );
        }
        if (yScroll < yScrollMax)
        {
            uiRenderer.Draw2DRectGradient(
                xOffset, uiState.scrH - Cfg::uiScrollIndicatorSize, menuBar.width, Cfg::uiScrollIndicatorSize,
                fadeColor, uiLayout.mainMenuColor[1]
            );
        }
    }

    // Draws UI elements and also interacts with user input immediately.
    void DrawUIImmediateMode()
    {
        // Returns true if either all hand actions are none or any of them is equal to the input action
        auto AllHandAction = [this](HandAction action) -> bool
        {
            return (user.hand[0].action == action && user.hand[1].action == action);
        };

        auto AnyHandAction = [this](HandAction action) -> bool
        {
            return (user.hand[0].action == action || user.hand[1].action == action);
        };

        // Draw mouse dead zone as transparent background for the UI
        uiRenderer.Draw2DRect(
            /*position:*/   0, uiLayout.mainMenuSize,
            /*size:*/       uiState.menuBarLeft.width, uiState.scrH - uiLayout.mainMenuSize,
            /*color:*/      Cfg::uiSideMenuColor);
        uiRenderer.Draw2DRect(
            /*position:*/   uiState.scrW - uiState.menuBarRight.width, uiLayout.mainMenuSize,
            /*size:*/       uiState.menuBarRight.width, uiState.scrH - uiLayout.mainMenuSize,
            /*color:*/      Cfg::uiSideMenuColor);

        // Draw entire color palette for the user to choose from on the left of the overlay
        uiState.ResetToMenuBar(uiLayout, uiState.menuBarLeft, uiLayout.colorPickerSize, true);

        const bool canSelectLeftMenu = AllHandAction(HandAction::None) || AnyHandAction(HandAction::LeftMenu_Capture) || AnyHandAction(HandAction::LeftMenu_Select);

        for_range(i, ColorPalette::Get().Count())
        {
            uiState.AdvanceColumn(uiLayout);
            {
                if (uiState.IsVisible())
                {
                    const bool isHighlighted = (user.pickedColorPaletteIndex == i);
                    if (unsigned pointerId = DrawUIColorPicker(uiState.x, uiState.y, i, canSelectLeftMenu, isHighlighted))
                    {
                        if (user.hand[pointerId - 1].action == HandAction::LeftMenu_Select)
                        {
                            // Set new color from palette to preview brick
                            previewInstance.colorIndex = i;
                            user.pickedColorPaletteIndex = i;
                        }
                    }
                }
            }
            uiState.AdvanceRow();
        }

        DrawScrollIndicator(uiState.menuBarLeft);

        // Draw brick thumbnails for the user to choose from on the right of the overlay
        uiState.ResetToMenuBar(uiLayout, uiState.menuBarRight, uiLayout.brickThumbnailSize, true);

        const bool canSelectRightMenu = AllHandAction(HandAction::None) || AnyHandAction(HandAction::RightMenu_Capture);

        for_range(brickId, BrickDictionary::Get().NumBrickTypes())
        {
            uiState.AdvanceColumn(uiLayout);
            {
                if (uiState.IsVisible())
                {
                    if (DrawUIBrickThumbnail(uiState.x, uiState.y, brickId, canSelectRightMenu))
                    {
                        // Set new brick type to preview brick
                        viewOptions.previewBrick.id = brickId;
                    }
                }
            }
            uiState.AdvanceRow();
        }

        DrawScrollIndicator(uiState.menuBarRight);

        // Draw main menu bar
        const bool canSelectMainMenu = (user.hand[0].action == HandAction::None && user.hand[1].action == HandAction::None);

        uiRenderer.Draw2DRectGradient(0, 0, uiState.scrW, uiLayout.mainMenuSize, uiLayout.mainMenuColor[0], uiLayout.mainMenuColor[1]);

        uiState.x = (uiState.scrW - uiState.mainMenuWidth)/2;
        uiState.y = (uiLayout.mainMenuSize - uiLayout.mainMenuItemSize - UILayout::buttonMargin)/2;

        uiState.advance = uiLayout.mainMenuBaseAdvance;

        #if LLGL_OS_WASM
        if (DrawUIMenuItem(UIMenuItem_Download, canSelectMainMenu))
        {
            const std::string timePointFormat = FormatTimePoint(std::chrono::system_clock::now());
            std::stringstream sceneContentStream;
            if (SaveSceneToStream(*currentScene, sceneContentStream))
                OfferFileDownload(sceneContentStream.str(), "BrickPlay_" + timePointFormat + ".txt");
        }
        #else
        (void)DrawUIMenuItem(UIMenuItem_Download, canSelectMainMenu, false, true);
        #endif

        DrawUIMenuItem(UIMenuItem_Upload, canSelectMainMenu);

        if (DrawUIMenuItem(UIMenuItem_Undo, canSelectMainMenu, false, history.IsAtBegining()))
            Undo();
        if (DrawUIMenuItem(UIMenuItem_Redo, canSelectMainMenu, false, history.IsAtEnd()))
            Redo();

        if (DrawUIMenuItem(UIMenuItem_TurnCW, canSelectMainMenu))
            user.menuGridRotation = static_cast<GridRotation>(ModuloSignInt(user.menuGridRotation + 1, GridRotation_Num));
        if (DrawUIMenuItem(UIMenuItem_TurnCCW, canSelectMainMenu))
            user.menuGridRotation = static_cast<GridRotation>(ModuloSignInt(user.menuGridRotation - 1, GridRotation_Num));
    }

    void PrepareUIDrawCommands()
    {
        uiRenderer.UpdateUIView(*commands, swapChain->GetResolution());

        uiRenderer.BeginBatched2DRendering(*commands);
        {
            DrawUIImmediateMode();
        }
        uiRenderer.EndBatched2DRendering();
    }

    //////////////////// USER INPUT HANDLING ////////////////////

    void Undo()
    {
        if (const History::Event* event = history.Undo())
        {
            // First, delete previously added brick
            if (event->addedBrickId != INVALID_BRICK_ID)
            {
                if (Brick addedBrick = currentScene->FindBrickAt(event->addedInstance.GetPos()))
                    currentScene->DestroyBrick(addedBrick);
            }

            // Then re-add previously destroyed brick. If this order is mixed up, they could overlap and only the 'destroy' event takes place.
            if (event->destroyedBrickId != INVALID_BRICK_ID)
            {
                currentScene->NewBrick(event->destroyedBrickId, event->destroyedInstance.GetPos(), event->destroyedInstance.gridRotation, event->destroyedInstance.colorIndex);
            }
        }
    }

    void Redo()
    {
        if (const History::Event* event = history.Redo())
        {
            // First, delete brick again
            if (event->destroyedBrickId != INVALID_BRICK_ID)
            {
                if (Brick destroyedBrick = currentScene->FindBrickAt(event->destroyedInstance.GetPos()))
                    currentScene->DestroyBrick(destroyedBrick);
            }

            // Then add new brick again. If this order is mixed up, they could overlap and only the 'destroy' event takes place.
            if (event->addedBrickId != INVALID_BRICK_ID)
            {
                currentScene->NewBrick(event->addedBrickId, event->addedInstance.GetPos(), event->addedInstance.gridRotation, event->addedInstance.colorIndex);
            }
        }
    }

    void StartDesktopCameraMode(HandAction newMode)
    {
        // Hide cursor and store its last position before starting camera movement mode
        user.lastCursorPos = LLGL::Display::GetCursorPosition();
        LLGL::Display::ShowCursor(false);
        user.hand[0].action = newMode;
    }

    void StopDesktopCameraMode()
    {
        // Show cursor and restore its last position when ending camera movement mode
        user.hand[0].action = HandAction::None;
        LLGL::Display::SetCursorPosition(user.lastCursorPos);
        LLGL::Display::ShowCursor(true);
    }

    void DesktopCameraControls(float dt)
    {
        switch (user.hand[0].action)
        {
            case HandAction::None:
            {
                if (input.KeyDown(LLGL::Key::RButton))
                    StartDesktopCameraMode(HandAction::Desktop_TurnCamera);
                else if (input.KeyDown(LLGL::Key::MButton))
                    StartDesktopCameraMode(HandAction::Desktop_MoveCamera);
            }
            break;

            case HandAction::Desktop_TurnCamera:
            {
                // Turn camera by mouse motion
                const LLGL::Offset2D motion = input.GetMouseMotion();
                const float pitch = static_cast<float>(+motion.y) * Cfg::cameraTurnSpeed;
                const float yaw   = static_cast<float>(-motion.x) * Cfg::cameraTurnSpeed;
                camera.Turn(pitch, yaw);

                if (input.KeyUp(LLGL::Key::RButton))
                    StopDesktopCameraMode();
            }
            break;

            case HandAction::Desktop_MoveCamera:
            {
                // Move camera by mouse motion
                const float cameraMoveCoefficient = (0.5f / 60.0f) / dt;
                const LLGL::Offset2D motion = input.GetMouseMotion();
                const float xAxis = static_cast<float>(+motion.x) * Cfg::cameraMoveSpeed * cameraMoveCoefficient;
                const float yAxis = static_cast<float>(-motion.y) * Cfg::cameraMoveSpeed * cameraMoveCoefficient;
                camera.Move({ xAxis, yAxis, 0 });

                if (input.KeyUp(LLGL::Key::MButton))
                    StopDesktopCameraMode();
            }
            break;

            default:
            break;
        }

        // Move camera by keyboard
        const float moveSpeed = (input.KeyPressed(LLGL::Key::Shift) ? Cfg::cameraMoveSpeed*3.0f : Cfg::cameraMoveSpeed);
        if (input.KeyPressed(LLGL::Key::A) || input.KeyPressed(LLGL::Key::Left))
            camera.Move({ -moveSpeed, 0, 0 });
        if (input.KeyPressed(LLGL::Key::D) || input.KeyPressed(LLGL::Key::Right))
            camera.Move({ +moveSpeed, 0, 0 });
        if (input.KeyPressed(LLGL::Key::W) || input.KeyPressed(LLGL::Key::Up))
            camera.Move({ 0, 0, +moveSpeed });
        if (input.KeyPressed(LLGL::Key::S) || input.KeyPressed(LLGL::Key::Down))
            camera.Move({ 0, 0, -moveSpeed });
    }

    void DesktopInputControls(float dt)
    {
        if (input.KeyDown(LLGL::Key::Tab))
            viewOptions.showWireframe = !viewOptions.showWireframe;

        if (input.KeyDown(LLGL::Key::F1))
            viewOptions.showMenu = !viewOptions.showMenu;

        DesktopCameraControls(dt);
    }

    GridVec3i WorldPosToGridPos(const Gs::Vector3f& worldPos, const GridVec3i& contactNormal, const BrickDimensions& brickDim)
    {
        constexpr float minOffset = 0.01f;

        auto AxisToOffset = [minOffset](int brickOffset, int brickExtent, int normal) -> float
        {
            const float centerOffset = static_cast<float>(1 - brickExtent % 2)*0.5f + (-brickOffset > brickExtent/2 ? 1.0f : 0.0f);
            const float baseOffset = minOffset * static_cast<float>(std::abs(normal));
            return (normal == 0 ? centerOffset + baseOffset : baseOffset);
        };

        const float xOffsetReal = AxisToOffset(brickDim.offset.x, brickDim.extent.x, contactNormal.x);
        const float yOffsetReal = minOffset;
        const float zOffsetReal = AxisToOffset(brickDim.offset.z, brickDim.extent.z, contactNormal.z);

        return GridVec3i
        {
            static_cast<std::int32_t>(std::floor(worldPos.x / KDTreeNode::gridScale.x + xOffsetReal)),
            static_cast<std::int32_t>(std::floor(worldPos.y / KDTreeNode::gridScale.y + yOffsetReal)),
            static_cast<std::int32_t>(std::floor(worldPos.z / KDTreeNode::gridScale.z + zOffsetReal))
        };
    }

    // Resets the previously picked brick and returns its instance.
    Instance* ResetPreviouslySelectedBrick()
    {
        // Reset previously picked brick
        if (Instance* oldBrickInstance = currentScene->GetMutableBrickInstance(user.pickedBrick))
        {
            oldBrickInstance->colorIndex = user.pickedInstanceColorIndex;
            user.pickedBrick = {};
            return oldBrickInstance;
        }
        return nullptr;
    }

    Brick SelectBrickFromScene(const LLGL::InputPointer& inputPointer)
    {
        // Pick scene objects with the mouse pointer
        const Ray3f pickerRay = ProjectScreenPosToViewRay(inputPointer.position);

        RayIntersection contact;
        if (currentScene->PickScene(pickerRay, contact))
        {
            // Highlight new picked brick
            if (Instance* newBrickInstance = currentScene->GetMutableBrickInstance(contact.brick))
            {
                user.pickedInstanceColorIndex   = newBrickInstance->colorIndex;
                user.pickedBrick                = contact.brick;

                newBrickInstance->colorIndex = ColorPalette::Get().HighlightColorIndex();
            }
            return contact.brick;
        }

        return Brick{};
    }

    void ClampBrickGridOffset(BrickId brickId, std::int32_t& offset)
    {
        const GridVec3i brickExtent = BrickDictionary::Get().GetExtent(brickId);
        const std::int32_t maxWidthOrDepth = std::max(brickExtent.x, brickExtent.z);
        if (maxWidthOrDepth > 2)
        {
            const std::int32_t maxOffset = (maxWidthOrDepth + 1)/2 - 1;
            offset = Gs::Clamp(offset, -maxOffset, +maxOffset);
        }
        else
            offset = 0;
    }

    void PickGridPosForPreviewBrick(const LLGL::InputPointer& inputPointer)
    {
        PreviewBrick& previewBrick = viewOptions.previewBrick;

        // Allow user to turn and change type of preview brick while picking the scene
        GridRotation newPreviewRotation = user.menuGridRotation;
        if (input.KeyDown(LLGL::Key::Space))
        {
            const bool turnCW = input.KeyPressed(LLGL::Key::Shift);
            newPreviewRotation = static_cast<GridRotation>(ModuloSignInt(newPreviewRotation + (turnCW ? +1 : -1), GridRotation_Num));
        }
        user.menuGridRotation = newPreviewRotation;

        BrickId newBrickId = previewBrick.id;
        if (input.KeyDownRepeated(LLGL::Key::PageUp))
            newBrickId = static_cast<BrickId>((newBrickId + 1) % BrickDictionary::Get().NumBrickTypes());
        if (input.KeyDownRepeated(LLGL::Key::PageDown))
            newBrickId = static_cast<BrickId>(ModuloSignInt(newBrickId - 1, BrickDictionary::Get().NumBrickTypes()));

        previewBrick.gridOffset += input.GetWheelMotion();

        ClampBrickGridOffset(newBrickId, previewBrick.gridOffset);

        // Pick scene objects with the mouse pointer
        const Ray3f pickerRay = ProjectScreenPosToViewRay(inputPointer.position);
        GridVec3i newPreviewGridPos = previewInstance.GetPos();

        RayIntersection contact;
        if (currentScene->PickScene(pickerRay, contact))
        {
            const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(newBrickId, newPreviewRotation);

            // Find a potential grid position for the preview brick
            auto ApplyNormalOffset = [](std::int32_t& gridPos, int normal, int brickOffset, int brickExtent) -> void
            {
                if (normal < 0)
                    gridPos -= (brickExtent + brickOffset);
                else if (normal > 0)
                    gridPos -= brickOffset;
            };

            for (int axis = 0; axis < 3; ++axis)
            {
                if (contact.normal[axis] != 0)
                {
                    newPreviewGridPos = WorldPosToGridPos(contact.point, contact.normal, brickDim);
                    ApplyNormalOffset(newPreviewGridPos[axis], contact.normal[axis], brickDim.offset[axis], brickDim.extent[axis]);
                    break;
                }
            }

            // Offset by user input to allow placing bricks on ledges
            if (brickDim.extent.z > brickDim.extent.x)
            {
                if (contact.normal.z == 0)
                    newPreviewGridPos.z += previewBrick.gridOffset;
            }
            else
            {
                if (contact.normal.x == 0)
                    newPreviewGridPos.x += previewBrick.gridOffset;
            }
        }

        // If grid position changed, re-evaluate whether that position is blocked for the preview brick
        if (previewInstance.GetPos()        != newPreviewGridPos    ||
            previewInstance.gridRotation    != newPreviewRotation   ||
            previewBrick.id                 != newBrickId)
        {
            previewBrick.isBlocked = !currentScene->BrickFitsInGrid(newBrickId, newPreviewGridPos, newPreviewRotation);

            // Update brick position and rotation
            previewBrick.id = newBrickId;
            previewInstance.SetPos(newPreviewGridPos);
            previewInstance.gridRotation = newPreviewRotation;
        }
    }

    bool NewBrickAndRegister(BrickId brickId, Instance brickInstance)
    {
        if (currentScene->NewBrick(brickId, brickInstance.GetPos(), brickInstance.gridRotation, brickInstance.colorIndex))
        {
            history.Register(History::Event{ brickId, brickInstance, INVALID_BRICK_ID, {} });
            return true;
        }
        return false;
    }

    // Event when the brick movement is canceled and the selected brick is set back to its previous location.
    void CancelBrickMoveEvent()
    {
        if (user.backupInstance.gridRotation != GridRotation_Num)
        {
            (void)NewBrickAndRegister(viewOptions.previewBrick.id, user.backupInstance);
            user.backupInstance.gridRotation = GridRotation_Num;
        }
    }

    // Event when the user adds a new brick. This is recorded in the event history so the user can revert it via undo/redo.
    void NewBrickEvent()
    {
        if (!NewBrickAndRegister(viewOptions.previewBrick.id, previewInstance))
            CancelBrickMoveEvent();
    }

    void DestroyBrickEvent(Brick brick)
    {
        if (const Instance* brickInstance = currentScene->GetBrickInstance(brick))
            history.Register(History::Event{ INVALID_BRICK_ID, {}, brick.GetId(), *brickInstance });

        currentScene->DestroyBrick(brick);
    }

    bool IsInputDown(const LLGL::InputPointer& inputPointer) const
    {
        return
        (
            (!IsRunningOnMobileDevice() && input.KeyDown(LLGL::Key::LButton)) ||
            (IsRunningOnMobileDevice() && inputPointer.state == LLGL::InputState::Activate)
        );
    }

    bool IsInputUp(const LLGL::InputPointer& inputPointer) const
    {
        return
        (
            (!IsRunningOnMobileDevice() && input.KeyUp(LLGL::Key::LButton)) ||
            (IsRunningOnMobileDevice() && inputPointer.state == LLGL::InputState::Deactivate)
        );
    }

    void ProcessInputPointerBrickMovement(UserHand& hand, const LLGL::InputPointer& inputPointer)
    {
        PickGridPosForPreviewBrick(inputPointer);

        if (!IsRunningOnMobileDevice() && input.KeyDown(LLGL::Key::RButton))
        {
            // Place brick back to its previous location and transition back to 'select' mode
            CancelBrickMoveEvent();
            hand.action = HandAction::None;
        }
        else if (IsInputUp(inputPointer))
        {
            // Place brick at new location and transition back to 'select' mode
            NewBrickEvent();
            hand.action = HandAction::None;
        }
    }

    void ProcessInputPointerBrickSelection(UserHand& hand, const LLGL::InputPointer& inputPointer)
    {
        if (Brick selectedBrick = SelectBrickFromScene(inputPointer))
        {
            if (IsInputDown(inputPointer))
            {
                // Pick up brick for editing so the user can place it somewhere else
                if (const Instance* selectedInstance = currentScene->GetBrickInstance(selectedBrick))
                {
                    ResetPreviouslySelectedBrick();

                    // Prepare new preview brick
                    viewOptions.previewBrick.id     = selectedBrick.GetId();
                    previewInstance.colorIndex      = selectedInstance->colorIndex;
                    previewInstance.gridRotation    = selectedInstance->gridRotation;
                    user.menuGridRotation           = selectedInstance->gridRotation;

                    // Also save current brick information in case the newly placed location is blocked and this brick is set back to its previous position
                    user.backupInstance = *selectedInstance;

                    // Try to find picked color in color palette. If not found, it was edited externally and we keep the color picker unchanged.
                    if (selectedInstance->colorIndex < ColorPalette::Get().Count())
                        user.pickedColorPaletteIndex = selectedInstance->colorIndex;

                    // Change to edit mode that allows the user to move the selected brick around.
                    // Immediately update picking position of the preview brick before it is rendered with a stale position.
                    hand.action = HandAction::MoveBrick;
                    PickGridPosForPreviewBrick(inputPointer);
                }

                // Remove selected brick
                if (user.pickedBrick == selectedBrick)
                    ResetPreviouslySelectedBrick();
                DestroyBrickEvent(selectedBrick);
            }
        }
    }

    bool IsPointerOverMenu(const LLGL::InputPointer& inputPointer) const
    {
        return viewOptions.showMenu &&
            ( inputPointer.position.y <= uiLayout.mainMenuSize      ||
              inputPointer.position.x < uiState.menuBarLeft.width   ||
              inputPointer.position.x > uiState.scrW - uiState.menuBarRight.width );
    }

    void ProcessInputPointerStartPhase(UserHand& hand, const LLGL::InputPointer& inputPointer)
    {
        if (inputPointer.position.y > uiLayout.mainMenuSize)
        {
            if (inputPointer.position.x <= uiState.menuBarLeft.width)
            {
                // Start input capture for left menu bar
                if (IsInputDown(inputPointer))
                    hand.Capture(HandAction::LeftMenu_Capture, inputPointer.position);
            }
            else if (inputPointer.position.x >= uiState.scrW - uiState.menuBarRight.width)
            {
                // Start input capture for right menu bar
                if (IsInputDown(inputPointer))
                    hand.Capture(HandAction::RightMenu_Capture, inputPointer.position);
            }
            else
            {
                // Select bricks from the scene
                ProcessInputPointerBrickSelection(hand, inputPointer);
            }
        }
    }

    // Decides whether the user intends to scroll the menu bar or pick a color
    void ProcessInputPointerDecideLeftMenuAction(UserHand& hand, const LLGL::InputPointer& inputPointer)
    {
        float moveVecY = static_cast<float>(inputPointer.position.y - hand.capturePos.y);

        if (std::abs(moveVecY) >= uiLayout.menuBarTriggerDist)
        {
            hand.action = HandAction::LeftMenu_Scroll;
            hand.captureScroll = uiState.menuBarLeft.scroll.position + static_cast<float>(hand.capturePos.y);
        }
        else if (IsInputUp(inputPointer))
            hand.action = HandAction::LeftMenu_Select;
    }

    void ProcessInputPointerDecideRightMenuAction(UserHand& hand, const LLGL::InputPointer& inputPointer)
    {
        float moveVecX = static_cast<float>(inputPointer.position.x - hand.capturePos.x);
        float moveVecY = static_cast<float>(inputPointer.position.y - hand.capturePos.y);

        // Prioritize pulling bricks out of the palette, so make sure Y-axis is clearly loinger than X-axis in the motion vector
        if (std::abs(moveVecY) >= uiLayout.menuBarTriggerDist && std::abs(moveVecY) > std::abs(moveVecX)*2.0f)
        {
            hand.action = HandAction::RightMenu_Scroll;
            hand.captureScroll = uiState.menuBarRight.scroll.position + static_cast<float>(hand.capturePos.y);
        }
        else if (inputPointer.position.x < uiState.scrW - uiState.menuBarRight.width)
        {
            // Initialize grid position to move brick
            hand.action = HandAction::MoveBrick;
            PickGridPosForPreviewBrick(inputPointer);
        }
    }

    void ProcessInputPointerMenuScroll(UserHand& hand, const LLGL::InputPointer& inputPointer, UIState::MenuBar& menuBar)
    {
        menuBar.scrollTarget = std::max(0.0f, hand.captureScroll - static_cast<float>(inputPointer.position.y));

        if (IsInputUp(inputPointer))
        {
            // Release input capture and give scroll bar a last spin when it still has momentum
            float remainingScrollDist = menuBar.scrollTarget - menuBar.scroll.position;
            menuBar.scroll.AddForces(remainingScrollDist * 5.0f);
            menuBar.scrollTarget = -1.0f;
            hand.action = HandAction::None;
        }
    }

    void ProcessInputPointerUpdatePhase(UserHand& hand, const LLGL::InputPointer& inputPointer)
    {
        switch (hand.action)
        {
            case HandAction::None:
                break; // dummy

            case HandAction::Mobile_MoveCamera:
                break;
            case HandAction::Mobile_TurnCamera:
                break;

            case HandAction::Desktop_MoveCamera:
                break; // ignore here
            case HandAction::Desktop_TurnCamera:
                break; // ignore here

            case HandAction::LeftMenu_Capture:
                ProcessInputPointerDecideLeftMenuAction(hand, inputPointer);
                break;
            case HandAction::LeftMenu_Scroll:
                ProcessInputPointerMenuScroll(hand, inputPointer, uiState.menuBarLeft);
                break;
            case HandAction::LeftMenu_Select:
                hand.action = HandAction::None;
                break;

            case HandAction::RightMenu_Capture:
                ProcessInputPointerDecideRightMenuAction(hand, inputPointer);
                break;
            case HandAction::RightMenu_Scroll:
                ProcessInputPointerMenuScroll(hand, inputPointer, uiState.menuBarRight);
                break;

            case HandAction::MoveBrick:
                ProcessInputPointerBrickMovement(hand, inputPointer);
                break;
        }
    }

    void ProcessInputPointer(UserHand& hand, const LLGL::InputPointer& inputPointer)
    {
        if (inputPointer.state != LLGL::InputState::Inactive)
        {
            hand.isOverMenu = IsPointerOverMenu(inputPointer);

            // If input pointer has not been captured yet, check whether it must be captured now
            if (hand.action == HandAction::None)
                ProcessInputPointerStartPhase(hand, inputPointer);
            else
                ProcessInputPointerUpdatePhase(hand, inputPointer);
        }
    }

    void ProcessUserInput(float dt)
    {
        if (!IsRunningOnMobileDevice())
            DesktopInputControls(dt);

        Instance* oldBrickInstance = ResetPreviouslySelectedBrick();

        for_range(pointerIndex, NumInputPointers())
            ProcessInputPointer(user.hand[pointerIndex], input.GetPointer(pointerIndex));

        camera.ApplyForces(dt, currentScene.get());

        #if KDTREE_DEBUG_LEVEL >= 1
        if (currentScene->GetBrickInstance(user.pickedBrick) != oldBrickInstance)
        {
            LLGL_LOG(
                "Selected brick (type=%u, instance=%u): %u Ray/AABB tests\n",
                user.pickedBrick.groupId, user.pickedBrick.groupInstance, KDTreeNode::numAABBTests
            );
        }
        #endif
    }

    void UpdateSceneView()
    {
        camera.UpdateTransform();

        // Update common scene view states
        sceneView.viewPos           = Gs::TransformVector(camera.wMatrix, Gs::Vector3f{});
        sceneView.vpMatrix          = projection * camera.vMatrix;

        sceneView.gridScale         = KDTreeNode::gridScale;

        sceneView.lightDir[0]       = Gs::Vector4f{ -0.2f, +0.4f, -1.0f, 0.0f }.Normalized();

        // Update preview shader states
      //sceneView.worldOffsetY      = previewBlockOffsetY;
        sceneView.isDithering       = (viewOptions.previewBrick.isBlocked ? 1 : 0);

        // Update outline shader states
        const LLGL::Extent2D& viewportSize = swapChain->GetResolution();
        sceneView.viewportSize[0]   = static_cast<float>(viewportSize.width);
        sceneView.viewportSize[1]   = static_cast<float>(viewportSize.height);

        // Update inverse view-projection matrix to displace depth values for parallax occlusion displacement
        sceneView.vpMatrixInv           = sceneView.vpMatrix.Inverse();
        sceneView.vpMatrixInvNoViewPos  = (projection * camera.vMatrixNoPosition).Inverse();
    }

    bool IsPreviewBrickVisible() const
    {
        for_range(pointerIndex, NumInputPointers())
        {
            if (user.hand[pointerIndex].action == HandAction::MoveBrick && !user.hand[pointerIndex].isOverMenu)
                return true;
        }
        return false;
    }

    //////////////////// MAIN CALLBACKS ////////////////////

    void OnDrawFrame(float dt) override
    {
        #if KDTREE_DEBUG_LEVEL
        KDTreeNode::numAABBTests = 0;
        #endif

        UpdateUILayout(dt);
        ProcessUserInput(dt);
        UpdateSceneView();

        const bool isPreviewBrickVisible = IsPreviewBrickVisible();

        commands->Begin();
        {
            // Update all brick thumbnails when the camera rotates on Y-axis or the preview rotation changed
            if (viewOptions.showMenu)
                RenderAllBrickThumbnailsIfChanged(user.menuGridRotation, camera.yaw);

            // Always update instance for preview brick if its visible
            if (isPreviewBrickVisible)
                commands->UpdateBuffer(*vertexAndPreviewInstanceBuffer, previewInstanceBufferOffset, &previewInstance, sizeof(previewInstance));

            commands->UpdateBuffer(*sceneViewCbuffer, 0, &sceneView, sizeof(sceneView));

            if (viewOptions.showMenu)
            {
                // Prepare UI draw commands by generating vertex data and uploading them to the GPU. Draw commands are batched and flushed in the final render-pass.
                commands->PushDebugGroup("PrepareUIDrawCommands");
                {
                    PrepareUIDrawCommands();
                }
                commands->PopDebugGroup();
            }

            // Update dynamic scene data
            UpdatePlaneGeometryForViewFrustum();

            currentScene->FlushInstanceDirtyRange(*commands);

            const LLGL::Viewport fullViewport{ swapChain->GetResolution() };

            // Draw opaque scene
            commands->BeginRenderPass(*opacity.renderTarget, opacity.renderPass, 1, opacity.clearValues);
            {
                // Clear all buffers. Stencil might be used for silhouette rendering.
                commands->SetViewport(fullViewport);

                commands->PushDebugGroup("RenderBrickScene");
                {
                    RenderBrickScene(*currentScene);
                }
                commands->PopDebugGroup();

                if (isPreviewBrickVisible)
                {
                    commands->PushDebugGroup("RenderPreviewBrick");
                    {
                        BindPreviewBrickPSOStates();
                        RenderPreviewBrick(viewOptions.previewBrick);
                    }
                    commands->PopDebugGroup();
                }
            }
            commands->EndRenderPass();

            // Draw transparent scene
            commands->BeginRenderPass(*transparency.renderTarget, transparency.renderPass, 2, transparency.clearValues);
            {
                commands->SetViewport(fullViewport);

                commands->PushDebugGroup("RenderTransparentBrickInstances");
                {
                    RenderTransparentBrickInstances(*currentScene);
                }
                commands->PopDebugGroup();
            }
            commands->EndRenderPass();

            // Draw overlay and UI elements
            commands->BeginRenderPass(*swapChain);
            {
                commands->SetViewport(fullViewport);

                commands->PushDebugGroup("RenderTransparentComposite");
                {
                    RenderTransparentComposite();
                }
                commands->PopDebugGroup();

                if (selectionRenderMode != SelectionRenderMode::Simplified && user.pickedBrick)
                {
                    commands->PushDebugGroup("RenderSilhouette");
                    {
                        RenderSilhouette(*currentScene, user.pickedBrick);
                    }
                    commands->PopDebugGroup();
                }

                if (viewOptions.showMenu)
                {
                    commands->PushDebugGroup("RenderUIElements");
                    {
                        uiRenderer.FlushDrawCommands(*commands);
                    }
                    commands->PopDebugGroup();
                }
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

    void OnResize(const LLGL::Extent2D& resolution) override
    {
        // Re-create all size dependent resources
        CreateSizeDependentResources(resolution);
    }

    void OnExit() override
    {
        // Only save file if it was loaded successfully.
        // We don't want to override an existing file with an empty scene because something went wrong at startup.
        #if !LLGL_OS_WASM
        if (hasLoadedScene)
        {
            // Ensure previoulsy selected brick retains its original color
            ResetPreviouslySelectedBrick();
            if (currentScene->HasUnsavedChanges() && !options.discardEdits)
                SaveSceneToFile(*currentScene, options.sceneFilename.c_str());
        }
        #endif
        currentScene.reset();
    }

};

const Gs::Vector3f  KDTreeNode::gridScale    = { 1.0f, 0.5f, 1.0f };
int                 KDTreeNode::numAABBTests = 0;
AABB3i              KDTreeNode::rootAABB;


LLGL_IMPLEMENT_EXAMPLE(Example_BrickPlay);



