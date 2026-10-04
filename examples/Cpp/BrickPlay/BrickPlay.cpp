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
#include <ExampleBase.h>
#include <PerlinNoise.h>
#include <FileUtils.h>
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Platform/Platform.h>
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

    static constexpr bool           useCameraAcceleration           = true;
    static constexpr float          cameraTurnSpeed                 = 0.25f;
    static constexpr float          cameraMoveSpeed                 = 25.0f;
    static constexpr float          cameraMass                      = 0.1f;     // Camera mass for physics based movement.
    static constexpr float          cameraDampingCeofficient        = 0.99f;
    static constexpr float          cameraCollisionRadius           = 1.5f;

    static constexpr float          previewBlockOffsetY             = 0.075f;   // Y-axis world position offset for the preview block before putting it into place

    static constexpr float          silhouetteColor[3]              = { 1.0f, 1.0f, 0.0f }; // Color of the silhouette outline for selectin bricks
    static constexpr float          silhouetteWidth                 = 5.0f;

    static constexpr int            uiColorPickerSize               = 26;       // Size (in pixels) of each color picker button
    static constexpr int            uiColorPickerOffset[2]          = { 15, 15 };
    static constexpr int            uiButtonFrameSize               = 3;
    static constexpr int            uiButtonMargin                  = 5;
    const LLGL::ColorRGBAub         uiButtonFrameColor              = { 0, 0, 0, 255 };

    static constexpr int            uiBrickThumbnailSize            = 48;
    const LLGL::ColorRGBAub         uiBrickThumbnailBgColor         = { 180, 180, 180, 255 };
    const LLGL::ColorRGBAub         uiMenuBgColor                   = { 0, 0, 0, 180 };

    static constexpr float          uiThumbnailMinVolumeSize        = 3.0f; // Minimum size for a bounding volume of brick thumbnails.

    static constexpr int            maxBrickExtent                  = 16;   // Limit how large bricks can be in X/Y/Z dimension

    static constexpr int            planeSubdivisions               = 1;

    static constexpr int            heightFieldChunkSize            = 2;    // Build bricks from height field in 4x4 grid chunks so that the largest bricks can be utilized

    // Whether to use extended brick models for their optimized versions (*_WallsOnlyExt.obj).
    // If false, covered bricks can show tiny holes due to beveled edges of bricks sitting above and below them.
    // This can be turned off for mobile platforms where this flaw is virtually invisible
    // and the incurred performance cost of more almost invisible geometry can be avoided.
    static constexpr bool           useWaterTightBrickOptimization  = true;

    // High precision format for transparency accumulation texture. Set this to either RGBA16Float or RGBA32Float.
    static constexpr LLGL::Format   transparencyAccumFormat         = LLGL::Format::RGBA16Float;

    struct Options
    {
        std::string                 sceneFilename                   = "Scenes/Bricks.scene.txt";
        std::string                 configFilename                  = "BrickPlay.config.txt";
        int                         scenePresetNo                   = -1; // Auto-generate preset scene
        int                         sceneBoundingVolume[3]          = { 100, 60, 100 };
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

    LLGL::Texture*                  uiBrickThumbnailTexture         = nullptr;

    LLGL::Sampler*                  uvWrapSampler                   = nullptr; // UV address wrap sampler. Anisotropic for desktop platforms.

    LLGL::Texture*                  skyboxCubemap                   = nullptr;

    bool                            hasOffsetInstancing             = false;
    bool                            hasLoadedScene                  = false;
    bool                            canCombineVertexAndIndexBuffers = false; // WebGL 2 cannot combine vertex and index buffers

    std::unique_ptr<BrickScene>     currentScene;

    UIRenderer                      uiRenderer;

    using ColorPalette      = BrickUtils::ColorPalette;

    using Instance          = BrickScene::Instance;
    using RayIntersection   = BrickScene::RayIntersection;

    Instance                        previewInstance                 = {};

    struct UIState
    {
        // To avoid competing mouse interactions between scene picking and UI elements,
        // declare a dead zone for the scene picker when the menu is active on the left and on the right.
        // This adjusted according to screen resolution when UI elements have to shift.
        int menuBarLeft     = 0;
        int menuBarRight    = 0;
    }
    uiState;

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
        float               outlineColor[3]         = { silhouetteColor[0], silhouetteColor[1], silhouetteColor[2] };
        float               outlineWidth            = silhouetteWidth;

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
        BrickId         id          = static_cast<BrickId>(0); // Previews brick with instance index 0.
        bool            isBlocked   = false;
        std::int32_t    gridOffset  = 0;
    };

    struct ViewOptions
    {
        bool            showWireframe               = false;
        PreviewBrick    previewBrick;
    }
    viewOptions;

    struct PlaneVertex
    {
        Gs::Vector3f worldPos;
    };

    struct PlaneModel
    {
        static constexpr int            maxNumFaces             = planeSubdivisions * planeSubdivisions;
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

    enum class CameraMode
    {
        None,
        TurnCamera,
        MoveCamera,
    };

    enum class EditMode
    {
        None,
        Add,
        Move,
        Remove,
    };

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

    // State tracking for user controls such as picking scene objects with the mouse pointer.
    struct Controller
    {
        CameraMode          cameraMode                  = CameraMode::None;
        EditMode            editMode                    = EditMode::None;

        LLGL::Offset2D      lastCursorPos;

        Brick               pickedBrick;
        int                 pickedInstanceColorIndex    = 0; // Color palette index of lasted picked brick

        int                 pickedColorPaletteIndex     = 0;
    }
    controller;

    struct Camera
    {
        Gs::Vector3f    forces;
        Gs::Vector3f    velocity;
        Gs::Vector3f    position;
        float           pitch       = 0.0f;
        float           yaw         = 0.0f;

        Gs::Matrix4f    wMatrix;
        Gs::Matrix4f    vMatrix;
        Gs::Matrix4f    vMatrixNoPosition;

        void Turn(float pitch, float yaw)
        {
            this->pitch = Gs::Clamp(this->pitch + pitch, -90.0f, +90.0f);
            this->yaw   = this->yaw + yaw;
        }

        void Move(const Gs::Vector3f& direction)
        {
            // Move camera into the direction, taking camera rotation into account
            Gs::Matrix3f rotation;
            Gs::RotateY(rotation, Gs::Deg2Rad(yaw));
            Gs::RotateX(rotation, Gs::Deg2Rad(pitch));
            forces += rotation * direction;

        }

        void ApplyForces(float dt, const BrickScene* collisionScene = nullptr)
        {
            if (useCameraAcceleration)
            {
                // Apply forces to velocity
                const Gs::Vector3f damping = -velocity * cameraDampingCeofficient;
                const Gs::Vector3f acceleration = (forces + damping) / cameraMass;
                velocity += acceleration * dt;
            }
            else
            {
                // Without physics integration, simply set velocity to input forces
                velocity = forces;
            }

            // Apply velocity to position and resolve collisions
            const Gs::Vector3f velocityDelta = velocity * dt;
            const float velocityDeltaLen = velocityDelta.Length();

            if (velocityDeltaLen > GS_EPSILON_F32)
            {
                // Resolve collisions in iterations by maximum velocity length
                if (collisionScene != nullptr)
                {
                    const float travelStep = cameraCollisionRadius * 0.9f;
                    const Gs::Vector3f moveDir = velocityDelta.Normalized();

                    float travelDist = 0.0f;
                    while (travelDist + travelStep < velocityDeltaLen)
                    {
                        position += moveDir * travelStep;
                        collisionScene->ResolveCollision(position, cameraCollisionRadius);
                        travelDist += travelStep;
                    }
                    position += moveDir * (velocityDeltaLen - travelDist);
                    collisionScene->ResolveCollision(position, cameraCollisionRadius);
                }
                else
                {
                    position += velocityDelta;
                }
            }

            // Reset forces
            forces = {};
        }

        void UpdateTransform()
        {
            wMatrix.LoadIdentity();
            Gs::Translate(wMatrix, position);
            Gs::RotateY(wMatrix, Gs::Deg2Rad(yaw));
            Gs::RotateX(wMatrix, Gs::Deg2Rad(pitch));

            vMatrix = wMatrix.Inverse();

            vMatrixNoPosition = Gs::Matrix4f
            {
                vMatrix(0, 0), vMatrix(0, 1), vMatrix(0, 2), 0.0f,
                vMatrix(1, 0), vMatrix(1, 1), vMatrix(1, 2), 0.0f,
                vMatrix(2, 0), vMatrix(2, 1), vMatrix(2, 2), 0.0f,
                         0.0f,          0.0f,          0.0f, 1.0f
            };
        }
    }
    camera;

public:

    Example_BrickPlay() :
        ExampleBase{ "LLGL Example: BrickPlay" }
    {
        // Initialize kd-Tree vounding volume and camera projection parameters
        InitAppConfig();

        // Create all graphics objects
        CreateResources();
        CreateSizeDependentResources(swapChain->GetResolution());
        if (!CreatePipelines())
            return;

        // Render all bricks into thumbnail render-targets for the UI
        RenderAllBrickThumbnails();

        // Load previoulsy saved scene or create default initial scene on failure.
        currentScene = std::unique_ptr<BrickScene>(new BrickScene(*renderer));
        if (options.scenePresetNo < 0)
        {
            if (LoadScene(*currentScene, options.sceneFilename.c_str()))
                hasLoadedScene = true;
        }
        else
        {
            CreateDefaultScene(*currentScene, options.scenePresetNo);
            hasLoadedScene = true;
        }

        currentScene->PrintInfo();

        // Resolve initial scene collision with camera
        currentScene->ResolveCollision(camera.position, cameraCollisionRadius);

        // Print info
        LLGL::Log::Printf(
            "Keyboard controls:\n"
            "  1 .............. Default mode\n"
            "  2 .............. Add brick mode\n"
            "  3/Delete ....... Move/Delete brick mode\n"
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
            LLGL::Log::Errorf("Failed to find file: %s\n", filename);
            return false;
        }

        LLGL::Blob fileContent = LLGL::Blob::CreateFromFile(filePath, true);
        if (!fileContent)
        {
            LLGL::Log::Errorf("Failed to read file: %s\n", filePath.c_str());
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

        const bool success = BrickUtils::ParseTextFile(
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
                        LLGL::Log::Errorf(
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
                    if (brickInfo.extent.x < 1 || brickInfo.extent.x > maxBrickExtent ||
                        brickInfo.extent.y < 1 || brickInfo.extent.y > maxBrickExtent ||
                        brickInfo.extent.z < 1 || brickInfo.extent.z > maxBrickExtent)
                    {
                        LLGL::Log::Errorf(
                            "Brick '%s' extent out of bounds [1, %d]: %d x %d x %d\n",
                            brickInfo.label.c_str(), maxBrickExtent, brickInfo.extent.x, brickInfo.extent.y, brickInfo.extent.z
                        );
                        return false;
                    }

                    // Load 3D mesh and emplace into group
                    std::string modelFilename, groupName;
                    stream >> modelFilename >> groupName;

                    BrickGroup brickGroup = (groupName.empty() ? BrickGroup_Default : BrickDictionary::Get().GroupFromString(groupName.c_str()));
                    if (brickGroup >= BrickGroup_Num)
                    {
                        LLGL::Log::Errorf("Invalid brick group '%s' in %s:%d\n", groupName.c_str(), filename, lineNo);
                        return false;
                    }
                    if (brickInfo.meshes[brickGroup].numVertices > 0)
                    {
                        LLGL::Log::Errorf(
                            "Warning: Brick '%s' overrides previous group model for '%s' in %s:%d\n",
                            brickInfo.label.c_str(), groupName.c_str(), filename, lineNo
                        );
                    }

                    // Expand upper bound of how many groups are used for this brick type
                    brickInfo.numGroups = std::max(brickInfo.numGroups, static_cast<std::uint32_t>(brickGroup) + 1);

                    if (!LoadBrickModelFile(brickInfo.meshes[brickGroup], modelFilename.c_str()))
                    {
                        LLGL::Log::Errorf("Failed to load brick model '%s' in %s:%d\n", modelFilename.c_str(), filename, lineNo);
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
                            LLGL::Log::Errorf(
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
                    ColorPalette::Get().Insert(BrickUtils::ColorFromString(colorStr));
                }
                else
                {
                    LLGL::Log::Errorf("Unknown command '%c' in %s:%d\n", cmd, filename, lineNo);
                    return false;
                }

                return true;
            }
        );
        if (!success)
            return false;

        LLGL::Log::Printf("Imported configuration file: %s\n", filename);
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
        LLGL::Log::Printf("Load brick model: %s\n", filename);

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

    void LoadAllDefaultBrickModels()
    {
#if 0
        enum ModelFlags
        {
            Model_IsWallOptimized       = (1 << 0), // Does the model has an optimized version that only renders its walls, with some potential but negligible holes?
            Model_IsWallOptimizedExt    = (1 << 1), // Does the model has an optimized version that only renders its walls, but is still 'water tight'?
            Model_IsGlassOptimized      = (1 << 2), // Does the model has a specialized version for transparent materials?
        };

        auto LoadBrickModelGroups = [this](const char* basename, BrickId brickId, int flags = 0) -> void
        {
            auto LoadBrickModel = [this, brickId](const char* name, BrickGroup brickGroup) -> void
            {
                BrickMesh& mesh = BrickDictionary::Get().GetMesh(brickId, brickGroup);
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
                if (useWaterTightBrickOptimization && (flags & Model_IsWallOptimizedExt) != 0)
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

        LoadBrickModelGroups("Brick1x1x1",  BrickType_1x1x1,        Model_IsWallOptimized);
        LoadBrickModelGroups("Tile1x1",     BrickType_1x1x1_Tile);
        LoadBrickModelGroups("Brick1x1x3",  BrickType_1x1x3,        Model_IsWallOptimized);
        LoadBrickModelGroups("Brick1x2x1",  BrickType_1x2x1,        Model_IsWallOptimizedExt);
        LoadBrickModelGroups("Brick1x2x3",  BrickType_1x2x3,        Model_IsWallOptimizedExt);
        LoadBrickModelGroups("Brick2x2x1",  BrickType_2x2x1,        Model_IsWallOptimizedExt);
        LoadBrickModelGroups("Tile2x2",     BrickType_2x2x1_Tile);
        LoadBrickModelGroups("Brick2x2x3",  BrickType_2x2x3,        Model_IsWallOptimizedExt | Model_IsGlassOptimized);
        LoadBrickModelGroups("Roof2x2x3",   BrickType_2x2x3_Roof);
        LoadBrickModelGroups("Roof2x3x3",   BrickType_2x3x3_Roof);
        LoadBrickModelGroups("Brick4x2x1",  BrickType_4x2x1,        Model_IsWallOptimizedExt);
        LoadBrickModelGroups("Brick4x2x3",  BrickType_4x2x3,        Model_IsWallOptimizedExt | Model_IsGlassOptimized);
#endif
    }

    void InitSceneBoundingVolume(const LLGL::ArrayView<int> boundingVolume)
    {
        KDTreeNode::InitBoundingVolume(boundingVolume);

        // Store grid boundaries as positive minimum and negative maximum so the shader can evaluate it in a single vector operation (see fragment shader 'PSPlane')
        sceneView.gridBounds[0] = +KDTreeNode::rootAABB.bbMin.x;
        sceneView.gridBounds[1] = +KDTreeNode::rootAABB.bbMin.z;

        sceneView.gridBounds[2] = -KDTreeNode::rootAABB.bbMax.x;
        sceneView.gridBounds[3] = -KDTreeNode::rootAABB.bbMax.z;

        LLGL::Log::Printf(
            "Scene bounding volume extent: %d x %d x %d\n",
            boundingVolume[0], boundingVolume[1], boundingVolume[2]
        );
    }

    void InitAppConfig()
    {
        // Allow user to override scene file
        if (ParseArgument("-preset", options.scenePresetNo))
            options.sceneFilename = "Bricks.Preset" + std::to_string(options.scenePresetNo) + ".txt";

        ParseArgument("-file", options.sceneFilename);

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
    }

    LLGL::Texture* CreateDummyCubeTexture()
    {
        LLGL::TextureDescriptor cubeTexDesc;
        {
            cubeTexDesc.debugName           = "DummyCubeTexture";
            cubeTexDesc.type                = LLGL::TextureType::TextureCube;
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
    LLGL::Texture* LoadCubeTexture(const std::string& basename, std::initializer_list<const char*> cubeFaces)
    {
        LLGL_VERIFY(cubeFaces.size() == 6);

        // Load individual cube faces
        LLGL::Texture* cubeFaceTextures[6] = {};
        int cubeFaceIndex = 0;
        LLGL::Extent3D cubeFace0Extent;
        constexpr long cubeFaceBindFlags = (LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment | LLGL::BindFlags::CopySrc);

        for (const char* cubeFaceName : cubeFaces)
        {
            // Load cube face texture
            cubeFaceTextures[cubeFaceIndex] = LoadTexture(basename + cubeFaceName, cubeFaceBindFlags);

            // Validate all cube faces have the same size and are square in size
            const LLGL::Extent3D cubeFaceExtent = cubeFaceTextures[cubeFaceIndex]->GetMipExtent(0);
            if (cubeFaceIndex > 0)
            {
                if (cubeFaceExtent != cubeFace0Extent)
                {
                    LLGL::Log::Errorf(
                        "Incompatible cube map sizes: face %d (size=%ux%ux%u)\n does not match face 0 (size=%ux%ux%u)\n",
                        cubeFaceIndex,
                        cubeFaceExtent.width, cubeFaceExtent.height, cubeFaceExtent.depth,
                        cubeFace0Extent.width, cubeFace0Extent.height, cubeFace0Extent.depth
                    );
                    return CreateDummyCubeTexture();
                }
            }
            else
            {
                if (cubeFaceExtent.width != cubeFaceExtent.height || cubeFaceExtent.depth != 1)
                {
                    LLGL::Log::Errorf(
                        "Incompatible cube map size: face 0 (size=%ux%ux%u) does not have a square size\n",
                        cubeFaceExtent.width, cubeFaceExtent.height, cubeFaceExtent.depth
                    );
                    return CreateDummyCubeTexture();
                }
                cubeFace0Extent = cubeFaceExtent;
            }
            ++cubeFaceIndex;
        }

        // Create cube texture with no initial data
        const std::string debugName = "CubeTexture[" + basename + "]";
        LLGL::TextureDescriptor cubeTexDesc;
        {
            cubeTexDesc.debugName       = debugName.c_str();
            cubeTexDesc.type            = LLGL::TextureType::TextureCube;
            cubeTexDesc.bindFlags       = LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment | LLGL::BindFlags::CopyDst;
            cubeTexDesc.extent          = cubeFace0Extent;
            cubeTexDesc.arrayLayers     = 6;
            cubeTexDesc.miscFlags       = LLGL::MiscFlags::NoInitialData;
        }
        LLGL::Texture* cubeTex = renderer->CreateTexture(cubeTexDesc);

        // Copy all cube faces into cube texture on the GPU and generate MIP-maps
        commands->Begin();
        {
            const std::string debugLabel = "BakeCubemap[" + basename + "]";
            commands->PushDebugGroup(debugLabel.c_str());
            {
                LLGL::TextureLocation dstLocation;
                const LLGL::TextureLocation srcLocation;

                for (std::uint32_t cubeFaceIndex = 0; cubeFaceIndex < 6u; ++cubeFaceIndex)
                {
                    dstLocation.arrayLayer = cubeFaceIndex;
                    if (LLGL::Texture* cubeFaceTex = cubeFaceTextures[cubeFaceIndex])
                        commands->CopyTexture(*cubeTex, dstLocation, *cubeFaceTex, srcLocation, cubeFace0Extent);
                }

                commands->GenerateMips(*cubeTex);
            }
            commands->PopDebugGroup();
        }
        commands->End();
        commandQueue->Submit(*commands);

        return cubeTex;
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
            #ifndef LLGL_MOBILE_PLATFORM
            samplerDesc.maxAnisotropy = 8;
            #endif
        }
        uvWrapSampler = renderer->CreateSampler(samplerDesc);

        // Load skybox textures
        skyboxCubemap = LoadCubeTexture("Skybox/sky_74_cubemap_2k/", { "px.png", "nx.png", "py.png", "ny.png", "pz.png", "nz.png" });

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
        if (!uiRenderer.Initialize(*renderer, *swapChain, vsOverlay, fsOverlay))
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
                psoOutlineDesc.rasterizer.lineWidth             = silhouetteWidth;
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

    bool SaveScene(BrickScene& scene, const char* filename)
    {
        // Open file for writing
        std::ofstream file{ filename };
        if (!file.good())
        {
            LLGL::Log::Errorf("Failed to save scene to file: %s\n", filename);
            return false;
        }

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
                file << "p " << BrickUtils::ColorToString(BrickUtils::Uint32ToColor(it.first)) << std::endl;
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
                auto it = colorToPaletteIndex.find(BrickUtils::ColorToUint32(color));
                if (it != colorToPaletteIndex.end())
                {
                    ::snprintf(colorStr, sizeof(colorStr), "p%d", it->second);
                    return colorStr;
                }
            }

            // Otherwise, write individual color
            return BrickUtils::ColorToString(color);
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

        LLGL::Log::Printf("Saved scene to file: %s\n", filename);
        return true;
    }

    bool LoadScene(BrickScene& scene, const char* filename)
    {
        std::stringstream file;
        if (!ReadFile(filename, file))
            return false;

        BrickId currentBrickType = INVALID_BRICK_ID;
        GridVec3i currentOffset;

        std::vector<LLGL::ColorRGBAub> intermediateColors;

        auto ColorOrPaletteIndexFromString = [&intermediateColors](const std::string& s) -> LLGL::ColorRGBAub
        {
            // Try to parse palette index first
            if (!s.empty() && s.front() == 'p')
            {
                int paletteIndex = std::atoi(&s[1]);
                if (paletteIndex < intermediateColors.size())
                    return intermediateColors[paletteIndex];

                LLGL::Log::Errorf("Color palette index %d out of bounds\n", paletteIndex);
                return {};
            }

            // Otherwise, parse individual color value
            return BrickUtils::ColorFromString(s);
        };

        // Parse every line and interpret it
        const bool success = BrickUtils::ParseTextFile(
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
                        LLGL::Log::Errorf("Missing brick type 'b' before new instance 'i' in %s:%d\n", filename, lineNo);
                        scene.ResetScene();
                        return false;
                    }

                    // Parse instance data
                    GridVec3i gridPos;
                    int gridRotation = 0;
                    std::string colorStr;

                    stream >> gridPos.x >> gridPos.y >> gridPos.z >> gridRotation >> colorStr;

                    if (gridRotation % 90 != 0)
                        LLGL::Log::Errorf("Ignored unknown brick rotation %d (must be a multiple of 90) in %s:%d\n", gridRotation, filename, lineNo);

                    // Find closest color from global palette
                    int colorIndex = ColorPalette::Get().FindClosestColorIndex(ColorOrPaletteIndexFromString(colorStr));

                    // Create new brick instance
                    scene.NewBrick(currentBrickType, gridPos + currentOffset, static_cast<GridRotation>(gridRotation / 90), colorIndex);
                }
                else if (cmd == 'b')
                {
                    // Parse brick type and make it current
                    std::string brickTypeStr;
                    stream >> brickTypeStr;
                    currentBrickType = BrickDictionary::Get().TypeFromString(brickTypeStr.c_str());

                    if (currentBrickType == INVALID_BRICK_ID)
                    {
                        LLGL::Log::Errorf("Unknown brick type '%s' in %s:%d\n", brickTypeStr.c_str(), filename, lineNo);
                        scene.ResetScene();
                        return false;
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
                        LLGL::Log::Errorf("Color palette entries must appear before brick types\n");
                        scene.ResetScene();
                        return false;
                    }

                    // Parse entry for intermediate color palette that's exclusive to this scene
                    std::string colorStr;
                    stream >> colorStr;
                    intermediateColors.push_back(BrickUtils::ColorFromString(colorStr));
                }
                else
                {
                    LLGL::Log::Errorf("Unknown command '%c' in %s:%d\n", cmd, filename, lineNo);
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

        LLGL::Log::Printf("Loaded scene from file: %s\n", filename);
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
        float field[heightFieldChunkSize][heightFieldChunkSize];

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

        LLGL::Log::Printf("Generated height field scene in %.03fs\n", static_cast<double>(elapsedSeconds) / 1000.0);
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

        for (int z = 0; z + heightFieldChunkSize < bbExtent.z; z += heightFieldChunkSize)
        {
            for (int x = 0; x + heightFieldChunkSize < bbExtent.x; x += heightFieldChunkSize)
            {
                int yRange[2] = { (heightFieldParams.yRange[0] / 3) * 3, heightFieldParams.yRange[1] };
                BuildBrickChunkFromHeightField(scene, x, z, heightFieldChunkSize, heightFieldChunkSize, yRange, noiseBuffer, bbExtent.x, brickId1x1, brickId1x1Tile);
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

        static_assert(planeSubdivisions > 0, "`planeSubdivisions` must be greater than 0");
        static_assert(planeSubdivisions <= 16, "`planeSubdivisions` should not be too large; consider keeping it at or below 16!");

        Gs::Vector3f segmentedVertices[planeSubdivisions + 1][planeSubdivisions + 1];

        const float planeSubdivisionsInv = 1.0f / static_cast<float>(planeSubdivisions);
        for (int x = 0; x <= planeSubdivisions; ++x)
        {
            float u = static_cast<float>(x) * planeSubdivisionsInv;

            const Gs::Vector3f horizontalTop    = Gs::Lerp(frustumLeftTop,    frustumRightTop,    u);
            const Gs::Vector3f horizontalBottom = Gs::Lerp(frustumLeftBottom, frustumRightBottom, u);

            for (int y = 0; y <= planeSubdivisions; ++y)
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
        for (int x = 0; x < planeSubdivisions; ++x)
        {
            for (int y = 0; y < planeSubdivisions; ++y)
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
            commands->SetPipelineState(*psoOutlineWriteStencil);
            commands->SetResource(0, *sceneViewCbuffer);
            commands->Draw(visibleMesh.numVertices, visibleMesh.firstVertex);
        }
        commands->PopDebugGroup();

        // Now render silhouette by rendering with a wireframe model with thick lines
        commands->PushDebugGroup("RenderSilhouetteColor");
        {
            commands->SetPipelineState(*psoOutlineWriteColor);
            commands->SetResource(0, *sceneViewCbuffer);
            commands->Draw(visibleMesh.numVertices, visibleMesh.firstVertex);
        }
        commands->PopDebugGroup();
    }

    void TransformSceneViewForThumbnail(BrickId brickId, GridRotation gridRotation, SceneView& outView)
    {
        const float dist = 25.0f;

        const BrickDimensions brickDim = BrickDictionary::Get().GetBrickDimensions(brickId, gridRotation);
        const GridVec3i brickCenter = (brickDim.offset*2 + brickDim.extent);
        const GridVec3i unitOffset{ brickCenter.x % 2, brickDim.extent.y, brickCenter.z % 2 };

        Gs::Matrix4f vMatrix;
        Gs::Translate(vMatrix, unitOffset.Cast<float>() * KDTreeNode::gridScale * 0.5f);
        Gs::RotateY(vMatrix, Gs::Deg2Rad(45.0f));
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
        const float volumeSize = std::max(uiThumbnailMinVolumeSize, maxBrickExtent + 1.0f);

        const Gs::Matrix4f thumbnailProjection = OrthogonalProjection(volumeSize, volumeSize, 0.1f, dist*2.0f);
        outView.vpMatrix = thumbnailProjection * vMatrix;
    }

    void RenderBrickThumnail(LLGL::RenderTarget& renderTarget, BrickId brickId, GridRotation gridRotation)
    {
        SceneView thumbnailSceneView;

        // Move camera so that the thumbnail for the current brick type is clearly visible
        TransformSceneViewForThumbnail(brickId, gridRotation, thumbnailSceneView);
        commands->UpdateBuffer(*sceneViewCbuffer, 0, &thumbnailSceneView, sizeof(thumbnailSceneView));

        commands->BeginRenderPass(renderTarget);
        {
            commands->SetViewport(LLGL::Extent2D{ uiBrickThumbnailSize, uiBrickThumbnailSize });
            commands->Clear(LLGL::ClearFlags::ColorDepth, LLGL::ClearValue{});

            // Draw current thumbnail brick from default group
            const BrickMesh mesh = BrickDictionary::Get().GetMesh(brickId);
            if (mesh.numVertices > 0)
                commands->Draw(mesh.numVertices, mesh.firstVertex);
        }
        commands->EndRenderPass();
    }

    void RenderAllBrickThumbnails()
    {
        const std::uint32_t numBrickTypes = BrickDictionary::Get().NumBrickTypes();

        // In case thumbnails must be updated, delete previous texture
        if (uiBrickThumbnailTexture != nullptr)
        {
            renderer->Release(*uiBrickThumbnailTexture);
            uiBrickThumbnailTexture = nullptr;
        }

        // Create thumbnail array texture
        LLGL::TextureDescriptor texArrayDesc;
        {
            texArrayDesc.debugName      = "BrickThumbnail.Texture2DArray";
            texArrayDesc.type           = LLGL::TextureType::Texture2DArray;
            texArrayDesc.extent         = { uiBrickThumbnailSize, uiBrickThumbnailSize, 1 };
            texArrayDesc.arrayLayers    = numBrickTypes;                    // Each brick type gets its own slot in the array texture
            texArrayDesc.mipLevels      = 1;                                // No MIP-maps since this is used for UI elements
            texArrayDesc.miscFlags      = LLGL::MiscFlags::NoInitialData;   // Don't initialize the texture since we render into it right below
        }
        uiBrickThumbnailTexture = renderer->CreateTexture(texArrayDesc);

        // Create intermediate render-targets, one for each array layer
        std::vector<LLGL::RenderTarget*> renderTargets;
        renderTargets.resize(numBrickTypes);

        for_range(brickId, numBrickTypes)
        {
            const std::string rtLabel = std::string("RenderTarget.Thumbnail_") + BrickDictionary::Get().TypeToString(brickId);
            LLGL::RenderTargetDescriptor rtDesc;
            {
                rtDesc.debugName                        = rtLabel.c_str();
                rtDesc.resolution                       = { uiBrickThumbnailSize, uiBrickThumbnailSize };
                rtDesc.samples                          = GetSampleCount();
                rtDesc.depthStencilAttachment.format    = swapChain->GetDepthStencilFormat();   // Use same depth format for thumbnails as for swap-chain (to make PSOs happy)

                // Render into array layer designated for current brick type
                if (rtDesc.samples > 1)
                {
                    rtDesc.colorAttachments[0].format       = LLGL::Format::RGBA8UNorm;
                    rtDesc.resolveAttachments[0].texture    = uiBrickThumbnailTexture;
                    rtDesc.resolveAttachments[0].arrayLayer = brickId;
                }
                else
                {
                    rtDesc.colorAttachments[0].texture      = uiBrickThumbnailTexture;
                    rtDesc.colorAttachments[0].arrayLayer   = brickId;
                }
            }
            renderTargets[brickId] = renderer->CreateRenderTarget(rtDesc);
        }

        // Reset preview instance to default
        constexpr GridRotation thumbnailRotation = GridRotation_270;

        previewInstance.SetPos({});
        previewInstance.gridRotation    = thumbnailRotation;
        previewInstance.colorIndex      = 0;

        // Render bricks as thumbnails into all offscreen render-targets
        commands->Begin();
        {
            commands->UpdateBuffer(*vertexAndPreviewInstanceBuffer, previewInstanceBufferOffset, &previewInstance, sizeof(previewInstance));

            BindPreviewBrickPSOStates();

            commands->PushDebugGroup("RenderBrickThumbnails");
            {
                for_range(brickId, numBrickTypes)
                {
                    const std::string debugLabel = std::string("RenderThumbnail_") + BrickDictionary::Get().TypeToString(brickId);
                    commands->PushDebugGroup(debugLabel.c_str());
                    {
                        RenderBrickThumnail(*renderTargets[brickId], brickId, thumbnailRotation);
                    }
                    commands->PopDebugGroup();
                }
            }
            commands->PopDebugGroup();
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Delete all intermediate render-targets, we only need the texture for UI rendering
        for (LLGL::RenderTarget* renderTarget : renderTargets)
            renderer->Release(*renderTarget);

        // Reset preview instance
        previewInstance = {};
    }

    bool IsMouseInRect(int x, int y, int w, int h)
    {
        const LLGL::Offset2D mousePos = input.GetMousePosition();
        return (mousePos.x >= x && mousePos.x <= x + w && mousePos.y >= y && mousePos.y <= y + h);
    }

    bool IsSelectedByMouse(int x, int y, int w, int h)
    {
        return (controller.cameraMode == CameraMode::None && IsMouseInRect(x, y, w, h));
    }

    bool DrawUIColorPicker(int x, int y, int paletteIndex, bool isHighlighted = false)
    {
        int w = uiColorPickerSize + uiButtonFrameSize*2;
        int h = uiColorPickerSize + uiButtonFrameSize*2;
        const bool isSelected = IsSelectedByMouse(x, y, w, h);

        LLGL::ColorRGBAub color = ColorPalette::Get()[paletteIndex];
        const UIRenderMode render = (color.a < 255 ? UIRenderMode_Dithered : UIRenderMode_Solid);

        if (isSelected || isHighlighted)
        {
            // Only draw color button when selected
            uiRenderer.Draw2DRect(x, y, w, h, color, render);
        }
        else
        {
            // Draw background frame and then the foreground color
            uiRenderer.Draw2DRect(x, y, w, h, uiButtonFrameColor);
            uiRenderer.Draw2DRect(x + uiButtonFrameSize, y + uiButtonFrameSize, uiColorPickerSize, uiColorPickerSize, color, render);
        }

        return (isSelected && input.KeyDown(LLGL::Key::LButton));
    }

    bool DrawUIBrickThumbnail(int x, int y, int brickType, bool isHighlighted = false)
    {
        int w = uiBrickThumbnailSize + uiButtonFrameSize*2;
        int h = uiBrickThumbnailSize + uiButtonFrameSize*2;
        const bool isSelected = IsSelectedByMouse(x, y, w, h);

        const LLGL::ColorRGBAub brickColor = ColorPalette::Get()[controller.pickedColorPaletteIndex];

        if (isSelected || isHighlighted)
        {
            // Draw button background to highlight selection
            uiRenderer.Draw2DRect(x, y, w, h, uiBrickThumbnailBgColor);
        }

        // Draw thumbnail texture layer
        uiRenderer.Draw2DImage(x + uiButtonFrameSize, y + uiButtonFrameSize, uiBrickThumbnailTexture, brickType, brickColor);

        return (isSelected && input.KeyDown(LLGL::Key::LButton));
    }

    // Draws UI elements and also interacts with user input immediately.
    void DrawUIImmediateMode()
    {
        const LLGL::Extent2D& resolution = swapChain->GetResolution();
        const int scrW = static_cast<int>(resolution.width);
        const int scrH = static_cast<int>(resolution.height);

        // Draw mouse dead zone as transparent background for the UI
        uiRenderer.Draw2DRect(0, 0, uiState.menuBarLeft, scrH, uiMenuBgColor);
        uiRenderer.Draw2DRect(uiState.menuBarRight, 0, scrW - uiState.menuBarRight, scrH, uiMenuBgColor);

        ResetMenuBar(swapChain->GetResolution());

        int uiPos[2], uiButtonAdvance;

        // Draw entire color palette for the user to choose from on the left of the overlay
        uiButtonAdvance = uiColorPickerSize + uiButtonFrameSize*2 + uiButtonMargin;
        uiPos[0] = uiColorPickerOffset[0];
        uiPos[1] = uiColorPickerOffset[1];

        for (int i = 0; i < ColorPalette::Get().Count(); ++i)
        {
            if (uiPos[1] > scrH - uiButtonAdvance)
            {
                // Move to next color picker column
                uiPos[0] += uiButtonAdvance;
                uiPos[1] = uiColorPickerOffset[1];
                uiState.menuBarLeft += uiButtonAdvance;
            }

            const bool isHighlighted = (controller.pickedColorPaletteIndex == i);
            if (DrawUIColorPicker(uiPos[0], uiPos[1], i, isHighlighted))
            {
                // Set new color from palette to preview brick
                previewInstance.colorIndex = i;
                controller.pickedColorPaletteIndex = i;
            }

            // Move to next UI element position
            uiPos[1] += uiButtonAdvance;
        }

        // Draw brick thumbnails for the user to choose from on the right of the overlay
        uiButtonAdvance = uiBrickThumbnailSize + uiButtonFrameSize*2 + uiButtonMargin;
        uiPos[0] = scrW - (uiColorPickerOffset[0] + uiBrickThumbnailSize + uiButtonFrameSize*2);
        uiPos[1] = uiColorPickerOffset[1];

        for_range(brickId, BrickDictionary::Get().NumBrickTypes())
        {
            if (uiPos[1] > scrH - uiButtonAdvance)
            {
                // Move to next color picker column
                uiPos[0] -= uiButtonAdvance;
                uiPos[1] = uiColorPickerOffset[1];
                uiState.menuBarRight -= uiButtonAdvance;
            }

            const bool isHighlighted = (viewOptions.previewBrick.id == brickId);
            if (DrawUIBrickThumbnail(uiPos[0], uiPos[1], brickId, isHighlighted))
            {
                // Set new brick type to preview brick
                viewOptions.previewBrick.id = brickId;
            }

            // Move to next UI element position
            uiPos[1] += uiButtonAdvance;
        }
    }

    void RenderUIElements()
    {
        uiRenderer.Begin2DRendering(*commands);
        {
            DrawUIImmediateMode();
        }
        uiRenderer.End2DRendering();
    }

    //////////////////// USER INPUT HANDLING ////////////////////

    void StartCameraMode(CameraMode newMode)
    {
        // Hide cursor and store its last position before starting camera movement mode
        controller.lastCursorPos = LLGL::Display::GetCursorPosition();
        LLGL::Display::ShowCursor(false);
        controller.cameraMode = newMode;
    }

    void StopCameraMode()
    {
        // Show cursor and restore its last position when ending camera movement mode
        controller.cameraMode = CameraMode::None;
        LLGL::Display::SetCursorPosition(controller.lastCursorPos);
        LLGL::Display::ShowCursor(true);
    }

    void ControlCameraByUserInput(float dt)
    {
        if (input.KeyDown(LLGL::Key::RButton) && controller.cameraMode == CameraMode::None)
            StartCameraMode(CameraMode::TurnCamera);
        else if (input.KeyUp(LLGL::Key::RButton) && controller.cameraMode == CameraMode::TurnCamera)
            StopCameraMode();

        if (input.KeyDown(LLGL::Key::MButton) && controller.cameraMode == CameraMode::None)
            StartCameraMode(CameraMode::MoveCamera);
        else if (input.KeyUp(LLGL::Key::MButton) && controller.cameraMode == CameraMode::MoveCamera)
            StopCameraMode();

        if (controller.cameraMode == CameraMode::TurnCamera)
        {
            // Turn camera by mouse motion
            const LLGL::Offset2D motion = input.GetMouseMotion();
            const float pitch = static_cast<float>(+motion.y) * cameraTurnSpeed;
            const float yaw   = static_cast<float>(-motion.x) * cameraTurnSpeed;
            camera.Turn(pitch, yaw);
        }
        else if (controller.cameraMode == CameraMode::MoveCamera)
        {
            // Move camera by mouse motion
            const float cameraMoveCoefficient = (0.5f / 60.0f) / dt;
            const LLGL::Offset2D motion = input.GetMouseMotion();
            const float xAxis = static_cast<float>(+motion.x) * cameraMoveSpeed * cameraMoveCoefficient;
            const float yAxis = static_cast<float>(-motion.y) * cameraMoveSpeed * cameraMoveCoefficient;
            camera.Move({ xAxis, yAxis, 0 });
        }

        // Move camera by keyboard
        const float moveSpeed = (input.KeyPressed(LLGL::Key::Shift) ? cameraMoveSpeed*3.0f : cameraMoveSpeed);
        if (input.KeyPressed(LLGL::Key::A) || input.KeyPressed(LLGL::Key::Left))
            camera.Move({ -moveSpeed, 0, 0 });
        if (input.KeyPressed(LLGL::Key::D) || input.KeyPressed(LLGL::Key::Right))
            camera.Move({ +moveSpeed, 0, 0 });
        if (input.KeyPressed(LLGL::Key::W) || input.KeyPressed(LLGL::Key::Up))
            camera.Move({ 0, 0, +moveSpeed });
        if (input.KeyPressed(LLGL::Key::S) || input.KeyPressed(LLGL::Key::Down))
            camera.Move({ 0, 0, -moveSpeed });
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
        if (Instance* oldBrickInstance = currentScene->GetMutableBrickInstance(controller.pickedBrick))
        {
            oldBrickInstance->colorIndex = controller.pickedInstanceColorIndex;
            controller.pickedBrick = {};
            return oldBrickInstance;
        }
        return nullptr;
    }

    Brick SelectBrickFromScene()
    {
        // Pick scene objects with the mouse pointer
        const Ray3f pickerRay = ProjectScreenPosToViewRay(input.GetMousePosition());

        RayIntersection contact;
        if (currentScene->PickScene(pickerRay, contact))
        {
            // Highlight new picked brick
            if (Instance* newBrickInstance = currentScene->GetMutableBrickInstance(contact.brick))
            {
                controller.pickedInstanceColorIndex = newBrickInstance->colorIndex;
                controller.pickedBrick              = contact.brick;

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

    void PickGridPosForPreviewBrick()
    {
        PreviewBrick& previewBrick = viewOptions.previewBrick;

        // Allow user to turn and change type of preview brick while picking the scene
        GridRotation newPreviewRotation = previewInstance.gridRotation;
        if (input.KeyDown(LLGL::Key::Space))
            newPreviewRotation = static_cast<GridRotation>((newPreviewRotation + 1) % GridRotation_Num);

        BrickId newBrickId = previewBrick.id;
        if (input.KeyDown(LLGL::Key::PageUp))
            newBrickId = static_cast<BrickId>((newBrickId + 1) % BrickDictionary::Get().NumBrickTypes());
        if (input.KeyDown(LLGL::Key::PageDown))
            newBrickId = static_cast<BrickId>(ModuloSignInt(newBrickId - 1, BrickDictionary::Get().NumBrickTypes()));

        previewBrick.gridOffset += input.GetWheelMotion();

        ClampBrickGridOffset(newBrickId, previewBrick.gridOffset);

        // Pick scene objects with the mouse pointer
        const Ray3f pickerRay = ProjectScreenPosToViewRay(input.GetMousePosition());
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

    void ProcessEditModeAddOrMove()
    {
        PickGridPosForPreviewBrick();

        const PreviewBrick& previewBrick = viewOptions.previewBrick;
        if (!previewBrick.isBlocked && input.KeyDown(LLGL::Key::LButton))
        {
            // Place new brick and update pick position for preview
            currentScene->NewBrick(previewBrick.id, previewInstance.GetPos(), previewInstance.gridRotation, previewInstance.colorIndex);
            PickGridPosForPreviewBrick();

            // Reset edit mode if it was only temporarily adding a new brick
            if (controller.editMode == EditMode::Move)
                controller.editMode = EditMode::Remove;
        }
    }

    void ProcessEditModeRemove()
    {
        if (Brick selectedBrick = SelectBrickFromScene())
        {
            if (input.KeyDown(LLGL::Key::LButton))
            {
                // Pick up brick for editing so the user can place it somewhere else
                if (const Instance* selectedInstance = currentScene->GetBrickInstance(selectedBrick))
                {
                    ResetPreviouslySelectedBrick();

                    viewOptions.previewBrick.id     = selectedBrick.GetId();
                    previewInstance.colorIndex      = selectedInstance->colorIndex;
                    previewInstance.gridRotation    = selectedInstance->gridRotation;

                    // Try to find picked color in color palette. If not found, it was edited externally and we keep the color picker unchanged.
                    if (selectedInstance->colorIndex < ColorPalette::Get().Count())
                        controller.pickedColorPaletteIndex = selectedInstance->colorIndex;

                    // Change to edit mode that allows the user to move the selected brick around.
                    // Immediately update picking position of the preview brick before it is rendered with a stale position.
                    controller.editMode = EditMode::Move;
                    PickGridPosForPreviewBrick();
                }

                // Remove selected brick
                if (controller.pickedBrick == selectedBrick)
                    ResetPreviouslySelectedBrick();
                currentScene->DestroyBrick(selectedBrick);
            }
        }
    }

    void ProcessUserInput(float dt)
    {
        const LLGL::Offset2D mousePos = input.GetMousePosition();
        const bool isMouseInDeadZone = (mousePos.x < uiState.menuBarLeft || mousePos.x > uiState.menuBarRight);

        if (input.KeyDown(LLGL::Key::Tab))
            viewOptions.showWireframe = !viewOptions.showWireframe;

        ControlCameraByUserInput(dt);

        if (input.KeyDown(LLGL::Key::D1))
            controller.editMode = EditMode::None;
        else if (input.KeyDown(LLGL::Key::D2))
            controller.editMode = EditMode::Add;
        else if (input.KeyDown(LLGL::Key::D3) || input.KeyDown(LLGL::Key::Delete))
            controller.editMode = EditMode::Remove;

        Instance* oldBrickInstance = ResetPreviouslySelectedBrick();

        // Only use mouse editing if the mouse isn't in the dead zone, i.e. covered by UI elements
        if (controller.cameraMode == CameraMode::None && !isMouseInDeadZone)
        {
            if (controller.editMode == EditMode::Add || controller.editMode == EditMode::Move)
                ProcessEditModeAddOrMove();
            else if (controller.editMode == EditMode::Remove)
                ProcessEditModeRemove();
        }

        camera.ApplyForces(dt, currentScene.get());

        #if KDTREE_DEBUG_LEVEL >= 1
        if (currentScene->GetBrickInstance(controller.pickedBrick) != oldBrickInstance)
        {
            LLGL::Log::Printf(
                "Selected brick (type=%u, instance=%u): %u Ray/AABB tests\n",
                controller.pickedBrick.groupId, controller.pickedBrick.groupInstance, KDTreeNode::numAABBTests
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

    void ResetMenuBar(const LLGL::Extent2D& resolution)
    {
        // Update mouse dead zone
        uiState.menuBarLeft     = uiColorPickerOffset[0]*2 + uiColorPickerSize + uiButtonFrameSize*2;
        uiState.menuBarRight    = static_cast<int>(resolution.width) - (uiColorPickerOffset[0]*2 + uiBrickThumbnailSize + uiButtonFrameSize*2);
    }

    //////////////////// MAIN CALLBACKS ////////////////////

    void OnDrawFrame(float dt) override
    {
        #if KDTREE_DEBUG_LEVEL
        KDTreeNode::numAABBTests = 0;
        #endif

        ProcessUserInput(dt);
        UpdateSceneView();

        const bool isPreviewBrickVisible = (controller.cameraMode == CameraMode::None && (controller.editMode == EditMode::Add || controller.editMode == EditMode::Move));

        commands->Begin();
        {
            // Always update instance for preview brick if its visible
            if (isPreviewBrickVisible)
                commands->UpdateBuffer(*vertexAndPreviewInstanceBuffer, previewInstanceBufferOffset, &previewInstance, sizeof(previewInstance));

            commands->UpdateBuffer(*sceneViewCbuffer, 0, &sceneView, sizeof(sceneView));

            uiRenderer.UpdateUIView(*commands, swapChain->GetResolution());

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

                if (controller.editMode == EditMode::Remove)
                {
                    commands->PushDebugGroup("RenderSilhouette");
                    {
                        RenderSilhouette(*currentScene, controller.pickedBrick);
                    }
                    commands->PopDebugGroup();
                }

                if (controller.editMode != EditMode::None)
                {
                    commands->PushDebugGroup("RenderUIElements");
                    {
                        RenderUIElements();
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
        if (hasLoadedScene)
        {
            // Ensure previoulsy selected brick retains its original color
            ResetPreviouslySelectedBrick();
            if (currentScene->HasUnsavedChanges())
                SaveScene(*currentScene, options.sceneFilename.c_str());
        }
        currentScene.reset();
    }

};

const Gs::Vector3f  KDTreeNode::gridScale    = { 1.0f, 0.5f, 1.0f };
int                 KDTreeNode::numAABBTests = 0;
AABB3i              KDTreeNode::rootAABB;


LLGL_IMPLEMENT_EXAMPLE(Example_BrickPlay);



